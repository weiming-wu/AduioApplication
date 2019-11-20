#ifndef __THREAD_H__
#define __THREAD_H__

#include "Process.h"
#include "Guard.h"
#include "MsgQue.h"
#include "Mutex.h"
#include "TaskQue.h"

#ifdef NO_MULTI_THREAD_SUPPORT
#error "please not define micro NO_MULTI_THREAD_SUPPORT"
#endif

class CThread
{
    friend int STDCALL ThreadBody(void *pdat);
public:
    CThread(BOOL bDeteched/*=FALSE*///这里主要是标示是否在线程体ThreadProc中进行自删除(delete this),不需要外部去清理线程资源,非线程池线程设置为FALSE*/
            , int nMsgQueSize/* = 0*/   //如果不使用消息队列,则设置为0(消息队列和任务队列只能同时使用其一,或都不用)
            , int nTaskQueSize/* = 0*/  //如果不使用任务队列,则设置为0(消息队列和任务队列只能同时使用其一,或都不用)
            , const char *name = ""
           );
    virtual ~CThread();
    BOOL CreateThread(int thread_priority = 0);
    virtual BOOL DestroyThread(BOOL bWaited = FALSE, BOOL bSignalQuit = TRUE/*调用本函数前已经调用过SignalQuit则传FALSE*/);
    BOOL IsThreadOver();
    int GetThreadID();
    BOOL SendMsg(uint msg, uint wpa = 0, ulong lpa = 0, int priority = MSG_PRIORITY_NORMAL);
    BOOL RecvMsg(XMSG *pMsg, BOOL wait = TRUE);
    BOOL PutTask(TASK *task, int priority = TASK_PRIORITY_NORMAL);
    BOOL GetTask(TASK *task, BOOL wait = TRUE);
    void SignalQuit(); //只通知线程需要退出,但是不等待线程退出及线程资源清理,调用该接口后一般再调用DestroyThread(TRUE,FALSE),用来加速同时退出多个线程
    void QuitMsg();
    void ClearMsg();
    uint GetMsgCount();
    virtual void ThreadProc() = 0;//线程执行体
    void SetTimeout(uint milliSeconds);
    BOOL IsTimeout();

protected:
    volatile BOOL   m_bLoop;//没有消息队列的线程/有消息队列线程发送XM_QUIT消息失败，使用这个变量控制线程结束，
    volatile BOOL   m_bWaitThreadExit;
    thread_t        m_Thread;
    int             m_ThreadID;
private:
    BOOL            m_bDeteched;
    CMsgQue        *m_pMsgQue;
    CTaskQue       *m_pTaskQue;

    CSemaphore      m_cSemaphore;   // 该信号量用来防止同一个对象的线程同时被创建多次；
    uint            m_expectedTime; // 预计执行结束时的时间，0表示不预计
    CSemaphore      m_desSemaphore;
    char            m_szThreadName[64];
    BOOL            m_bRunning; //线程是否正在运行
    CMutex          m_Mutex;
};

#endif //__THREAD_H__

