#include "TaskQue.h"
#include "Guard.h"

#ifdef HAS_HI_MEM_H
#include "hi_mem.h"
#else
#include <string.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////CTaskNode
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskNode
{
    friend class CTaskNodeManager;
protected:
    TASK                m_Task;//任务
    CTaskNode          *m_lpNext;//指向下一个节点
public:
    CTaskNode(): m_lpNext(NULL) {}
    virtual ~CTaskNode() {}
public:
    inline void             Init()
    {
        memset(&m_Task, 0, sizeof(m_Task));
        m_lpNext = NULL;
    }
    inline TASK            *GetTask()
    {
        return &m_Task;
    }
    void                    SetTask(const TASK &task)
    {
        memcpy(&m_Task, &task, sizeof(m_Task));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////CTaskNodeManager
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskNodeManager
{
public:
    CTaskNodeManager()
        : m_UsedTaskNode(0)
        , m_lpOriginalTaskList(NULL)
        , m_lpUsedTaskHead(NULL)
        , m_lpUsedTaskTail(NULL)
        , m_lpFreeTaskList(NULL)
    {

    }
    ~CTaskNodeManager()
    {
        CTaskNode *p = m_lpOriginalTaskList, *tmp;
        while (p)
        {
            tmp = p->m_lpNext;
            delete []p;
            p = tmp;
        }
    }
protected:
    CSemaphore          m_Semaphore;
    CMutex              m_Mutex;
    uint                m_UsedTaskNode;
    CTaskNode          *m_lpOriginalTaskList;
    CTaskNode          *m_lpUsedTaskHead;
    CTaskNode          *m_lpUsedTaskTail;
    CTaskNode          *m_lpFreeTaskList;
    enum { emTaskGroup = 64,};
    CTaskNode          *AllocTask()
    {
        CTaskNode *p = new CTaskNode[emTaskGroup];
        if (p)
        {
            for (int i = emTaskGroup - 1; i > 1; i--)
            {
                PutFreeTask(&p[i]);
            }

            p[0].m_lpNext = m_lpOriginalTaskList;
            m_lpOriginalTaskList = &p[0];

            p[1].Init();
            return &p[1];
        }
        return NULL;
    }
public:
    CTaskNode          *GetFreeTask()
    {
        CGuard guard(m_Mutex);
        if (!m_lpFreeTaskList)
        {
            return AllocTask();
        }
        CTaskNode *p   = m_lpFreeTaskList;
        if (m_lpFreeTaskList)
        {
            m_lpFreeTaskList    = m_lpFreeTaskList->m_lpNext;
        }
        p ->Init();
        return p;
    }
    void                PutUsedTask(CTaskNode *task, int priority)
    {
        if (task)
        {
            m_Mutex.Enter();
            if (TASK_PRIORITY_HIGHTEST == priority)
            {
                task ->m_lpNext = m_lpUsedTaskHead;
                m_lpUsedTaskHead = task;
                if (!m_lpUsedTaskTail)
                {
                    m_lpUsedTaskTail = task;
                }
            }
            else
            {
                if (m_lpUsedTaskTail)
                {
                    m_lpUsedTaskTail ->m_lpNext = task;
                }
                m_lpUsedTaskTail = task;
                if (!m_lpUsedTaskHead)
                {
                    m_lpUsedTaskHead = task;
                }
            }
            m_UsedTaskNode++;
            m_Mutex.Leave();
            m_Semaphore.Post();
        }
    }
    CTaskNode          *GetUsedTask(BOOL bWait)
    {
        /**
        *本函数隐患:
        *在使用RemoveUsedTask移除特定任务节点后,由于m_Semaphore.Wait()先执行,所以可能会返回NULL,不会100%成功
        *解决方案:
        *使用本函数时,必须判断返回值
        *问题解决:
        *后续再改进
        */
        if (bWait)
        {
            m_Semaphore.Wait();
        }
        m_Mutex.Enter();
        if (m_lpUsedTaskHead && m_UsedTaskNode > 0)
        {
            CTaskNode *p = m_lpUsedTaskHead;
            m_lpUsedTaskHead = m_lpUsedTaskHead ->m_lpNext;
            if (!m_lpUsedTaskHead)
            {
                m_lpUsedTaskTail = NULL;
            }
            m_UsedTaskNode--;
            m_Mutex.Leave();
            if (!bWait)
            {
                m_Semaphore.Wait();
            }
            return p;
        }
        m_Mutex.Leave();
        return NULL;
    }
    CTaskNode          *GetUsedTask(uint priv, BOOL bWait)
    {
        if (bWait)
        {
            m_Semaphore.Wait();
        }

        m_Mutex.Enter();
        CTaskNode *p =  m_lpUsedTaskHead, *prev = NULL;
        while (p)
        {
            if (p->m_Task.priv == priv)
            {
                //从队列移除
                if (m_lpUsedTaskHead == p)
                {
                    m_lpUsedTaskHead = m_lpUsedTaskHead ->m_lpNext;
                    if (!m_lpUsedTaskHead)
                    {
                        m_lpUsedTaskTail = NULL;
                    }
                }
                else if (prev)
                {
                    prev ->m_lpNext = p ->m_lpNext;
                    if (m_lpUsedTaskTail == p) //为最后一个节点
                    {
                        m_lpUsedTaskTail = prev;
                        m_lpUsedTaskTail ->m_lpNext = NULL;
                    }
                }
                m_UsedTaskNode--;
                m_Mutex.Leave();

                if (!bWait)
                {
                    m_Semaphore.Wait();
                }
                return p;
            }
            prev = p;
            p = p->m_lpNext;
        }
        m_Mutex.Leave();

        return NULL;
    }
    uint                GetUsedTaskNodeCount()
    {
        CGuard guard(m_Mutex);
        return m_UsedTaskNode;
    }
    void                PutFreeTask(CTaskNode *task)
    {
        CGuard guard(m_Mutex);
        if (task)
        {
            task->m_lpNext  = m_lpFreeTaskList;
            m_lpFreeTaskList = task;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////CTaskQue
////////////////////////////////////////////////////////////////////////////////////////////////////
CTaskQue::CTaskQue(uint maxTasks): m_nMaxTasks(maxTasks), m_lpTaskNodeManager(NULL)
{
    m_lpTaskNodeManager = new CTaskNodeManager();
}

CTaskQue::~CTaskQue(void)
{
    delete m_lpTaskNodeManager;
}

TaskToken_t             CTaskQue::PutTask(TASK *task, int priority)
{
    do
    {
        if (m_lpTaskNodeManager)
        {
            if (m_lpTaskNodeManager ->GetUsedTaskNodeCount() > m_nMaxTasks) //避免内存无限制增长
            {
                printf("CTaskQue::PutTask() Max Tasks Reached.\n");
                break;
            }
            CTaskNode *node = m_lpTaskNodeManager ->GetFreeTask();
            if (!node)
            {
                printf("CTaskQue::PutTask() Get Free Node Failed.\n");
                break;
            }
            node ->SetTask(*task);
            m_lpTaskNodeManager ->PutUsedTask(node, priority);
            //printf("ADD TASK:%d\n",m_lpTaskNodeManager ->GetUsedTaskNodeCount());
            return (TaskToken_t)node;
        }
    }
    while (0);
    return (TaskToken_t)0;
}

BOOL                    CTaskQue::GetTask(TASK *task, BOOL bWait)
{
    do
    {
        if (m_lpTaskNodeManager)
        {
            CTaskNode *node = m_lpTaskNodeManager ->GetUsedTask(bWait);
            if (!node)
            {
                if (bWait)
                {
                    printf("CTaskQue::GetTask() Error.\n");
                }
                break;
            }
            memcpy(task , node->GetTask(), sizeof(TASK));
            m_lpTaskNodeManager ->PutFreeTask(node);
            //printf("GET TASK:%d\n",m_lpTaskNodeManager ->GetUsedTaskNodeCount());
            return TRUE;
        }
    }
    while (0);
    return FALSE;
}

BOOL                    CTaskQue::RemoveTaskByPrivData(uint priv, SPECTASKPROC proc)
{

    CTaskNode *node = m_lpTaskNodeManager->GetUsedTask(priv, FALSE);
    while (node)
    {
        if (proc)
        {
            (proc)(node->GetTask());
        }
        m_lpTaskNodeManager ->PutFreeTask(node);
        node = m_lpTaskNodeManager->GetUsedTask(priv, FALSE);
    }
    return TRUE;
}

