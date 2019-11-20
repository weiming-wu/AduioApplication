#include <syscall.h>
#include <string.h>
#include <string.h>

#include "Thread.h"
#include "BaseFunction.h"

int STDCALL ThreadBody(void *pdat)
{
    CThread *pThread = (CThread *)pdat;
    BOOL bDeteched = pThread->m_bDeteched;

    pThread->m_ThreadID = syscall(__NR_gettid);

    if (pThread->m_szThreadName[0])
    {
        SetTaskName(pThread->m_szThreadName);
    }

    pThread->ThreadProc();

    if (!bDeteched)//解决线程池线程自删除问题
    {
        pThread->ClearMsg();
        pThread->m_cSemaphore.Post();

        if (pThread->m_bWaitThreadExit)
        {
            pThread->m_desSemaphore.Post();
        }
    }

    //ThreadExit();
    return 0;
}

CThread::CThread(BOOL bDeteched/*=FALSE*/, int nMsgQueSize /* = 0 */, int nTaskQueSize /* = 0 */, const char *name)
    : m_bDeteched(bDeteched)
    , m_pMsgQue(NULL)
    , m_pTaskQue(NULL)
    , m_cSemaphore(1)
    , m_desSemaphore(0)
    , m_bRunning(FALSE)
{
    memset(&m_Thread, 0, sizeof(m_Thread));
    m_ThreadID = 0;
    m_bLoop = FALSE;
    if (nMsgQueSize > 0)
    {
        m_pMsgQue = new CMsgQue(nMsgQueSize);
    }

    if (nTaskQueSize > 0)
    {
        m_pTaskQue = new CTaskQue(nTaskQueSize);
    }

    m_expectedTime = 0;

    m_bWaitThreadExit = TRUE;

    strcpy(m_szThreadName, name);
}

CThread::~CThread()
{
    delete m_pMsgQue;
    delete m_pTaskQue;
}

BOOL CThread::CreateThread(int thread_priority)
{
    int ret;
    if (!IsThreadOver())
    {
        return TRUE;
    }
    //如果已经创建，则等待退出后再创建
    m_cSemaphore.Wait();

    m_bLoop = TRUE;
    m_bWaitThreadExit = TRUE;

    if (thread_priority != 0)
    {
        pthread_attr_t tattr;
        struct sched_param tparam;
        pthread_attr_init(&tattr);
        pthread_attr_setschedpolicy(&tattr, SCHED_FIFO);
        pthread_attr_getschedparam(&tattr, &tparam);
        tparam.sched_priority = thread_priority;
        pthread_attr_setschedparam(&tattr, &tparam);
        ret = pthread_create(&m_Thread, &tattr, (void *(*)(void *))ThreadBody, this);
    }
    else
    {
        ret = thread_create(&m_Thread, ThreadBody, this);
    }
    if (ret == 0)
        //if( thread_create(&m_Thread, ThreadBody, this) == 0 )
    {
        //thread_getid(m_Thread,&m_ThreadID);

        //printf("++++++++++ID1 = %d\n",m_ThreadID);
        //m_ThreadID = thread_self();
        //printf("++++++++++ID2 = %d\n",m_ThreadID);
        m_bRunning = TRUE;

        if (m_bDeteched)
        {
            m_bWaitThreadExit = FALSE;
#ifdef OS_WINDOWS
            CloseHandle(m_Thread.handle);
            m_Thread.handle = NULL;
#else
            pthread_detach(m_Thread);
#endif
        }
        return TRUE;
    }
    else
    {
        perror("thread_create()");
    }
    return FALSE;
}

void CThread::SignalQuit()
{
    if (!IsThreadOver())
    {
        m_bLoop = FALSE;

        SendMsg(XM_QUIT);//如果发送出错，说明队列已满，此时m_bLoop可以保证线程循环退出
        TASK task;
        task.type = TASKTYPE_QUIT;
        PutTask(&task);
    }
}

BOOL CThread::DestroyThread(BOOL bWaited, BOOL bSignalQuit)
{
    if (!IsThreadOver())
    {
        int id = syscall(__NR_gettid);

        if (m_ThreadID != id)
        {
            m_bWaitThreadExit = bWaited;
        }
        //DBG("THREAD ID-->%d %d\n", m_ThreadID, id);

        if (bSignalQuit)
        {
            SignalQuit();
        }

        // 不是自己关自己的时候才采用阻塞式
        if (m_ThreadID != id)
        {
            if (m_bWaitThreadExit)
            {
                m_desSemaphore.Wait();
                thread_destroy(m_Thread);
            }
        }

        m_bRunning = FALSE;
    }
    return TRUE;
}

BOOL CThread::IsThreadOver()
{
    return !m_bRunning;//!m_bLoop;
}

int CThread::GetThreadID()
{
    return m_ThreadID;
}

BOOL CThread::SendMsg(uint msg, uint wpa /* = 0 */, ulong lpa /* = 0 */, int priority /* = 0 */)
{
    //发送消息
    if (m_pMsgQue)
    {
        return m_pMsgQue->SendMsg(msg, wpa, lpa, priority);
    }
    return FALSE;
}

BOOL CThread::RecvMsg(XMSG *pMsg, BOOL wait /* = TRUE */)
{
    //接收消息
    if (m_pMsgQue)
    {
        return m_pMsgQue->RecvMsg(pMsg, wait);
    }
    return FALSE;
}

void CThread::QuitMsg()
{
    //停止消息
    if (m_pMsgQue)
    {
        m_pMsgQue->QuitMsg();
    }
}

void CThread::ClearMsg()
{
    if (m_pMsgQue)
    {
        m_pMsgQue->ClearMsg();
    }
}

uint32 CThread::GetMsgCount()
{
    if (m_pMsgQue)
    {
        return m_pMsgQue->GetMsgCount();
    }
    return 0;
}

void CThread::SetTimeout(uint milliSeconds)
{
    if (milliSeconds == 0) // 清空预期时间
    {
        m_expectedTime = 0;
    }
    else
    {
        m_expectedTime = SystemGetMSCount() + milliSeconds;

        if (m_expectedTime < milliSeconds) // 计数溢出， 暂时不设置预期时时间
        {
            m_expectedTime = 0;
        }
    }
}

BOOL CThread::IsTimeout()
{
    return (m_expectedTime != 0 && m_expectedTime < SystemGetMSCount());
}

BOOL CThread::PutTask(TASK *task, int priority)
{
    if (m_pTaskQue)
    {
        return m_pTaskQue ->PutTask(task, priority) ? TRUE : FALSE;
    }
    return FALSE;
}

BOOL CThread::GetTask(TASK *task, BOOL wait)
{
    if (m_pTaskQue)
    {
        return m_pTaskQue ->GetTask(task, wait);
    }
    return FALSE;
}

