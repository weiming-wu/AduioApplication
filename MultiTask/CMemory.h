#ifndef __C_MEMORY_h__
#define __C_MEMORY_h__

#include <time.h>
#include <stdio.h>

#include "Type.h"
#include "Mutex.h"
#include "SysLog.h"

template <class T>
class CMemoryManager
{
public:
    T *GetNode();
    void PutNode(T *node);
    void PutNodeList(T *node);
    void PutNodeList(T *head, T *tail);

public:

    CMemoryManager(int groupCount = emGroupCount)
    {
        m_lpFreeNode = NULL;
        m_lpOrigBuff = NULL;
        m_GroupCount = groupCount;
    }

    ~CMemoryManager()
    {
        delete m_lpOrigBuff;
    }

private:
    T *Alloc();

private:
    enum { emGroupCount = 16};

    int m_GroupCount;
    T *m_lpFreeNode;
    COriginalBufferArray<T> *m_lpOrigBuff;
    CMutex m_Mutex;
};

template <class T>
T *CMemoryManager<T>::GetNode()
{
    CGuard g(m_Mutex);
    if (!m_lpFreeNode)
    {
        return Alloc();
    }

    T *p = m_lpFreeNode;
    m_lpFreeNode = m_lpFreeNode->next;
    memset(p, 0, sizeof(T));
    return p;
}

template <class T>
void CMemoryManager<T>::PutNode(T *node)
{
    m_Mutex.Enter();
    node->next = m_lpFreeNode;
    m_lpFreeNode = node;
    m_Mutex.Leave();
}

template <class T>
void CMemoryManager<T>::PutNodeList(T *node)
{
    for (T *p = node; p;)
    {
        if (p->next)
        {
            p = p->next;
            continue;
        }
        m_Mutex.Enter();
        p->next = m_lpFreeNode;
        m_lpFreeNode = node;
        m_Mutex.Leave();
        return;
    }
}

template <class T>
void CMemoryManager<T>::PutNodeList(T *head, T *tail)
{
    if (head && tail)
    {
        m_Mutex.Enter();
        tail->next = m_lpFreeNode;
        m_lpFreeNode = head;
        m_Mutex.Leave();
    }
}

template <class T>
T *CMemoryManager<T>::Alloc()
{
    uint32 count = !m_lpOrigBuff ? m_GroupCount : emGroupCount;
    T *p = new T[count];
    if (!p)
    {
        LOG(ERROR, "out of memory");
        return NULL;
    }

    LOG(INFO, "****Alloc:%u, bytes = %d\n", count, sizeof(T)*count);
    memset(p, 0, sizeof(T)*count);

    m_lpOrigBuff = new COriginalBufferArray<T>(p, m_lpOrigBuff);
    for (int i = (count - 1); i > 1; i--)
    {
        p[i].next = &p[i - 1];
    }
    m_lpFreeNode = &p[count - 1];
    return &p[0];
}

#endif //__C_MEMORY_h__

