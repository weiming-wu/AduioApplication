#ifndef __AUDIO_MSG_MANAGER_H__
#define __AUDIO_MSG_MANAGER_H__
#include <iostream>
#include <string>
#include "Thread.h"
#include "LuaOperate.h"
using namespace std;

enum
{
    LUA_HND_IDX_STEP = 0,
    LUA_HND_IDX_FINAL = 1,
    LUA_HND_IDX_MAX   = 2,
};

class CAudioMsgManager: public CThread
{
    PATTERN_SINGLETON_DECLARE(CAudioMsgManager);
public:
    BOOL Init();
    BOOL CleanUp();
    virtual void ThreadProc();
    static int AudioMsgReady(const void * raw_data, void * priv);
    RETURN_TYPE_E OnAudioMsgReady(const void * raw_data);
protected:
    CAudioMsgManager();
    ~CAudioMsgManager();
    virtual RETURN_TYPE_E AudioMsgProcess(const void *msg, uint32 len=0);
protected:
    lua_State *m_luaHnd[LUA_HND_IDX_MAX];
};

class CAudioMsgSend: public CThread
{
    PATTERN_SINGLETON_DECLARE(CAudioMsgSend);
public:
    BOOL Init();
    BOOL CleanUp();
    virtual void ThreadProc();
    /*
    RETURN_TYPE_E RegisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context);
    RETURN_TYPE_E UnregisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context);
    */
protected:
    CAudioMsgSend();
    ~CAudioMsgSend();

protected:
    //CCallbackInfo<AVDATA_CALLBACK_FUN> m_lpAVDataNotifier;
};

#define g_AudioMsgManager (*CAudioMsgManager::instance())

#define g_AudioMsgSend (*CAudioMsgSend::instance())


#endif

