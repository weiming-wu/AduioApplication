#ifndef __MUTEX_H__
#define __MUTEX_H__
#include <assert.h>
#include "Type.h"
#include "Sync.h"
#include "BaseFunction.h"

class CMutex
{
public:
    ALWAYS_INLINE CMutex()
    {
        int ret = locker_create(&m_Mutex) ;
        assert(0 == ret);
    };
    ALWAYS_INLINE ~CMutex()
    {
        locker_destroy(&m_Mutex);
    };
    ALWAYS_INLINE BOOL Enter()
    {
        return locker_lock(&m_Mutex) == 0 ? TRUE : FALSE;
    };
    ALWAYS_INLINE BOOL Leave()
    {
        return locker_unlock(&m_Mutex) == 0 ? TRUE : FALSE;
    };
protected:
private:
    locker_t m_Mutex;
};

#endif //__MUTEX_H__

