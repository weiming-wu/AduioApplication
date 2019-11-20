#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <assert.h>
#include "Sync.h"

class CSemaphore
{
public:
    inline CSemaphore(long lInitialCount = 0)
    {
        int ret = semaphore_create(&m_Semphore, NULL, lInitialCount);
        assert(0 == ret);
    };
    inline ~CSemaphore()
    {
        semaphore_destroy(&m_Semphore);
    };
    inline int Wait()
    {
        int ret;
        while (0 != (ret = semaphore_wait(&m_Semphore)))
        {
            if (errno == EINTR)
            {
                continue;
            }
            return ret;
        }
        return 0;
    };
    inline int Post()
    {
        return semaphore_post(&m_Semphore);
    };
private:
    semaphore_t m_Semphore;
};


#endif //__SEMAPHORE_H__

