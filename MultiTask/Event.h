#ifndef __EVENT_H__
#define __EVENT_H__

#include "Sync.h"
#include "Type.h"
class CEvent
{
public:
    CEvent(BOOL bAutoReset=FALSE/*是否自动复位*/,BOOL bInitialState=FALSE/*初始是否有信号*/)
    : m_AutoReset(bAutoReset)
    {
        event_create(&m_Event);
        if (bInitialState)
        {
            SetEvent();
        }
    }
    ~CEvent()
    {
        event_destroy(&m_Event);
    }

    BOOL Wait()
    {
        return 0==event_wait(&m_Event, m_AutoReset);
    }

    BOOL WaitTimeout(int s)
    {
        return 0==event_timewait(&m_Event, s, m_AutoReset);
    }

    void SetEvent()
    {
        if (m_AutoReset)
            event_signal(&m_Event);
        else
            event_broadcast(&m_Event);
    }

    void ResetEvent()
    {
        event_reset(&m_Event);
    }
private:
    BOOL        m_AutoReset;
    event_t     m_Event;
};

#endif //__EVENT_H__

