#include "MsgQue.h"
#include "Guard.h"

CMsgQue::CMsgQue(uint32 size /* = 1024 */)
{
    /************************************************************************
        消息队列的初始化工作：
        1、一次性把队列的所有消息结构体对象都创建起来;
        2、把这些对象都加到空闲队列m_FreeQueue;
        3、把消息队列可访问标志置为有效；
    ************************************************************************/
    m_nMaxMsg = size;
    m_nMsg = 0;
    m_bMsgFlg = TRUE;
}

CMsgQue::~CMsgQue()
{

}

BOOL CMsgQue::SendMsg(uint32 msg, uint32 wpa /* = 0 */, ulong lpa /* = 0 */, uint32 priority /* = 0 */)
{
    XMSG l_MSG;
    MSGQUEUE::iterator pi;

    m_Mutex.Enter();
    if (m_nMsg >= m_nMaxMsg)
    {
        m_Mutex.Leave();
        return FALSE;
    }
    if (priority >= MSG_PRIORITY_ILL)
    {
        m_Mutex.Leave();
        return FALSE;
    }
    if (!m_bMsgFlg)
    {
        m_Mutex.Leave();
        return FALSE;
    }

    /************************************************************************
        发送消息:
        1、按照优先级把该消息插入队列m_Queue；
    //  2、在该队列中查找该消息节点，直到找不到该节点才退出循环并从该函数返回；
    ************************************************************************/
    l_MSG.msg   = msg;
    l_MSG.wpa   = wpa;
    l_MSG.lpa   = lpa;
    l_MSG.time  = SystemGetMSCount();

    //现先插入，如果有必须要实现优先级，则再根据优先级排序插入
    if (priority == MSG_PRIORITY_HIGHTEST)
    {
        m_Queue.push_back(l_MSG);
    }
    else
    {
        m_Queue.push_front(l_MSG);
    }

    m_nMsg++;

    m_Mutex.Leave();

    m_Semaphore.Post();
    return TRUE;
}

BOOL CMsgQue::RecvMsg(XMSG *pMsg, BOOL wait /* = TRUE */)
{
    /************************************************************************
        接收消息，如果等待则一直等到有消息时返回，否则直接返回。
        1、从消息忙队列m_Queue取元素，如果取成功，则直接返回；
        2、否则循环从消息忙m_Queue中取元素，直到取成功才退出循环；
    ************************************************************************/
    if (wait)
    {
        m_Semaphore.Wait();
    }

    CGuard guard(m_Mutex);

    if (m_Queue.empty())
    {
        return FALSE;
    }
    if (!wait)
    {
        m_Semaphore.Wait();
    }
    assert(m_nMsg);
    *pMsg = m_Queue.back();
    m_Queue.pop_back();
    m_nMsg--;

    return TRUE;
}

void CMsgQue::QuitMsg()
{
    CGuard guard(m_Mutex);

    m_bMsgFlg = FALSE;
}

void CMsgQue::ClearMsg()
{
    CGuard guard(m_Mutex);

    uint32 n = m_nMsg;
    for (uint32 i = 0; i < n; i++)
    {
        m_Semaphore.Wait();
        m_Queue.pop_back();
        m_nMsg--;
    }
}

uint32 CMsgQue::GetMsgCount()
{
    CGuard guard(m_Mutex);

    return m_nMsg;
}

uint32 CMsgQue::GetMsgSize()
{
    CGuard guard(m_Mutex);

    return m_nMaxMsg;
}

void CMsgQue::SetMsgSize(uint32 size)
{
    CGuard guard(m_Mutex);

    m_nMaxMsg = size;
}


