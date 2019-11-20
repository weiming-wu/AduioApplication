#ifndef __GUARD_H__
#define __GUARD_H__

#include "Type.h"
#include "Mutex.h"

#ifndef NO_MULTI_THREAD_SUPPORT
class CGuard
{
public:
    ALWAYS_INLINE CGuard(CMutex &Mutex)
        : m_Mutex(Mutex)
    {
        m_Mutex.Enter();
    };

    ALWAYS_INLINE ~CGuard()
    {
        m_Mutex.Leave();
    };
private:
    CMutex &m_Mutex;
};

#else //NO_MULTI_THREAD_SUPPORT

class CGuard
{
public:
    ALWAYS_INLINE CGuard(CMutex &Mutex)
    {
    }
    ALWAYS_INLINE ~CGuard()
    {
    }
private:
};

#endif //NO_MULTI_THREAD_SUPPORT

#endif //__GUARD_H__

