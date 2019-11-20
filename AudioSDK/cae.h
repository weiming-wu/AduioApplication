#ifndef __CAE_H__
#define __CAE_H__

#include "cae_intf.h"
#include "BaseFunction.h"
#include "Thread.h"
#include "Type.h"
//#ifdef __cplusplus
//extern "C"{
//#endif
#include <alsa/asoundlib.h>

#if 0
/* ------------------------------------------------------------------------
** Global CAE Variable Definitions
** ------------------------------------------------------------------------ */
static Proc_CAENew api_cae_new;
static Proc_CAEAudioWrite api_cae_audio_write;
static Proc_CAEResetEng api_cae_reset_eng;
static Proc_CAESetRealBeam api_cae_set_real_beam;
static Proc_CAESetWParam api_cae_set_wparam;
//static Proc_CAEGetWParam api_cae_get_wparam;
static Proc_CAEGetVersion api_cae_get_version;
static Proc_CAEGetChannel api_cae_get_channel;
//static Proc_CAESetShowLog api_cae_set_show_log;
static Proc_CAEDestroy api_cae_destroy;


// Cae_Library_Path for wake up _ cae_LoadLibrary()
const char* kCaeLibPATH = "/usr/lib/libcae.so";

// The word of wake up resource _ api_cae_new()
const char* kIwvResPath = "/etc/ivw_resource.jet";

// msc login params _ MSPLogin()
// don't change it
const char* kMSCLoginParams = "appid = 58bcdd98, work_dir = .";

const char* kIatSessionParams =
        "sub = iat, domain = iat, language = zh_cn, "
        "accent = mandarin, sample_rate = 16000, "
        "result_type = plain, result_encoding = utf8";

const char* kJsonQuestionNotFound = "未找到问题答案";
#endif
typedef void* CAE_MODULEHANDLE;
typedef void* CAE_LIBHANDLE;

typedef struct _CAEUserData
{
    /*
    bool wakeUpState;//indicated is wakeup
    bool newSession;//indicated is new iat session
    bool mscInProcess;//indicated tts is in process
    bool suspendFlag;//used for tts interrupt
    bool wakeUping;//indicated wakeup is in progress
    char curFunc[20];//indicated current function,music,readbook,bt,or something
    */
    BOOL newSession;//indicated is new iat session
    BOOL wakeUping;//indicated wakeup is in progress
    BOOL wakeUpState;//indicated is wakeup
} CAEUserData;

//CAE_LIBHANDLE cae_LoadLibrary(const char* lib_name);

//int cae_FreeLibrary(CAE_LIBHANDLE lib_handle);

//void *cae_GetProcAddress(CAE_LIBHANDLE lib_handle, const char* fun_name);


//#ifdef __cplusplus
//}
//#endif

class CCAE: public CThread
{
    PATTERN_SINGLETON_DECLARE(CCAE);
public:
    BOOL Init();
    BOOL CleanUp();
    virtual void ThreadProc();
    static CAE_LIBHANDLE cae_LoadLibrary(const char* lib_name);
    static int cae_FreeLibrary(CAE_LIBHANDLE lib_handle);
    static void *cae_GetProcAddress(CAE_LIBHANDLE lib_handle, const char* fun_name);
    static int StateChange(int state, void * priv);
    RETURN_TYPE_E OnStateChange(int state);
private:
    int initCaeFuncs();
    BOOL MSP_Login(const char *loginParams);
    void CaeAudioWrite();
    static void CAEIvwCb(short angle, short channel, float power,
                    short CMScore, short beam, char *param1,
                    void *param2, void *userData);
    static void CAEAudioCb(const void *audioData, unsigned int audioLen,
                            int param1, const void *param2, void *userData);
    BOOL AlsaInit();
    void alsa_open(snd_pcm_t** capture_handle, int channels,
               uint32_t rate, snd_pcm_format_t format);
    int preProcessBuffer(void *data, void *out, int bytes);
protected:
    CCAE();
    ~CCAE();
private:
    CAE_HANDLE m_lpCaeHnd;
    CAEUserData m_UserData;
    snd_pcm_t *m_lpCaptureHnd;
    snd_pcm_format_t m_tFormat;
    uint32 m_nRate;
    int m_nChn;
};

#define g_CAE    (*CCAE::instance())
#endif