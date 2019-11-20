#ifndef __AIUI_IMPL_H__
#define __AIUI_IMPL_H__

#include <string>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include "AIUI.h"
#include "BaseFunction.h"
#include "FileUtil.h"
#include "Type.h"
#if defined(_PLAYBACK)
#include <alsa/asoundlib.h>
#endif
#define TEST_ROOT_DIR "./AIUI/"

//配置文件打的路径，里面是客户端设置的参数
#define CFG_FILE_PATH "./AIUI/cfg/aiui.cfg"

//测试音频的路径
#define TEST_AUDIO_PATH "./AIUI/audio/test.pcm"

#define GRAMMAR_FILE_PATH "./AIUI/asr/call.bnf"

#define LOG_DIR "./AIUI/log"

typedef int (*AVDATA_CALLBACK_FUN)(const void * pkt, void * context);
typedef int (*STATE_CHANGE_CALLBACK_FUN)(int state, void * context);

using namespace aiui;
using namespace std;

class CListenerImpl: public IAIUIListener
{
public:
	CListenerImpl();
	~CListenerImpl();
	void onEvent(const IAIUIEvent& event) const;
    void RegisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context);
    void UnregisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context);

    void RegisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context);
    void UnregisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context);

protected:
    CCallbackInfo<AVDATA_CALLBACK_FUN> m_lpAVDataNotifier;
    CCallbackInfo<STATE_CHANGE_CALLBACK_FUN> m_cbStateNotifier;
	FileUtil::DataFileHelper *m_lpTtsFileHelper;
};

class CAIUIImpl
{
    //单例模式
    PATTERN_SINGLETON_DECLARE(CAIUIImpl);
public:
    BOOL Init();
    BOOL CleanUp();
    void RegisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void *context);
    void UnregisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void *context);
    void RegisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context);
    void UnregisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context);
    BOOL Start();
    BOOL Stop();
    void CreateAgent();
    RETURN_TYPE_E WriteAudioData(const char *data, uint32 len);
    RETURN_TYPE_E WriteText();
    RETURN_TYPE_E Wakeup();
    RETURN_TYPE_E SyncSchema();//上传动态实体
    RETURN_TYPE_E QuerySyncStatus();//查询数据同步状态
#if defined (_DEBUG)
    void showIntroduction(bool detail);
    void readCmd();
    void test();
    void startTts();
#endif
protected:
    CAIUIImpl();
    ~CAIUIImpl();
private:
    IAIUIAgent *m_lpAgent;
    CListenerImpl m_Listener;
};

#define g_AIUIImpl    (*CAIUIImpl::instance())
#endif