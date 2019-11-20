#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
//#include <alsa/asoundlib.h>
#include "cae.h"
#include "msp_errors.h"
#include "msp_cmn.h"
#include "SysLog.h"
#include "AIUIImpl.h"
#include "iAIUIImpl.h"
#include "AlsaCapture.h"
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

const char* kCaeLibPATH = "/usr/lib/libcae.so";

const char* kIwvResPath = "/etc/ivw_resource.jet";

const char* kMSCLoginParams = "appid = 58bcdd98, work_dir = .";

PATTERN_SINGLETON_IMPLEMENT(CCAE);

CCAE::CCAE()
    : CThread(FALSE, 128, 0, "cae")
    , m_lpCaeHnd(NULL)
    , m_lpCaptureHnd(NULL)
    , m_tFormat(SND_PCM_FORMAT_S16_LE)
    , m_nRate(16000)
    , m_nChn(8)
{
    memset(&m_UserData, 0, sizeof(CAEUserData));
}

CCAE::~CCAE()
{

}

CAE_LIBHANDLE CCAE::cae_LoadLibrary(const char* lib_name)
{
#if defined(WIN32)
    return LoadLibrary( lib_name );
#else
    return dlopen(lib_name, RTLD_LAZY);
#endif
}

int CCAE::cae_FreeLibrary(CAE_LIBHANDLE lib_handle)
{
#if defined(WIN32)
    return FreeLibrary( lib_handle );
#else
    dlclose(lib_handle);
    return 1;
#endif
}
void * CCAE::cae_GetProcAddress(CAE_LIBHANDLE lib_handle, const char *fun_name)
{
#if defined(WIN32)
    return (void *)GetProcAddress( lib_handle, fun_name );
#else
    return dlsym(lib_handle, fun_name);
#endif
}

int CCAE::initCaeFuncs()
{
    int ret = MSP_SUCCESS;
    void* hInstance = cae_LoadLibrary(kCaeLibPATH);

    if(hInstance == NULL)
    {
        LOG(ERROR, "load cae library failed with path: %s\n", kCaeLibPATH);
        return MSP_ERROR_OPEN_FILE;
    }

    api_cae_new = (Proc_CAENew)cae_GetProcAddress(hInstance, "CAENew");
    api_cae_audio_write =
        (Proc_CAEAudioWrite)cae_GetProcAddress(hInstance, "CAEAudioWrite");
    api_cae_reset_eng =
        (Proc_CAEResetEng)cae_GetProcAddress(hInstance, "CAEResetEng");
    api_cae_set_real_beam =
        (Proc_CAESetRealBeam)cae_GetProcAddress(hInstance, "CAESetRealBeam");
    api_cae_set_wparam =
        (Proc_CAESetWParam)cae_GetProcAddress(hInstance, "CAESetWParam");
    api_cae_get_version =
        (Proc_CAEGetVersion)cae_GetProcAddress(hInstance, "CAEGetVersion");
    api_cae_get_channel=
        (Proc_CAEGetChannel)cae_GetProcAddress(hInstance, "CAEGetChannel");
    api_cae_destroy =
        (Proc_CAEDestroy)cae_GetProcAddress(hInstance, "CAEDestroy");

    return ret;
}

void CCAE::CAEIvwCb(short angle, short channel, float power,
                    short CMScore, short beam, char *param1,
                    void *param2, void *userData)
{
    LOG(INFO, "CAEIvwCb .... angle:%d, channel:%d, power:%f, CMScore: %d, beam: %d\n",angle, channel, power, CMScore, beam);

    CAEUserData *usDta = (CAEUserData*)userData;

    usDta->wakeUping = TRUE;
    usDta->newSession = TRUE;
    if (!usDta->wakeUpState)
    {
        usDta->wakeUpState = TRUE;

        /* 唤醒AIUI, wwm + 2019/10/11 */
        g_AIUIImpl.Wakeup();
    }
    //结束本次语音识别
    //QISRSessionEnd(iatSessionId, NULL);

    usleep(200 * 1000);
    usDta->wakeUping = FALSE;
}

void CCAE::CAEAudioCb(const void *audioData, unsigned int audioLen,
                        int param1, const void *param2, void *userData)
{
    //BOOL audioPadding;
    CAEUserData *usDta = (CAEUserData*)userData;
    if(usDta->wakeUping)
    {
        LOG(INFO, "---CAEAudioCb waiting for wakeup word finished---\n");
        return;
    }

    if (!usDta->wakeUpState)
    {
        //LOG("wakeUpState is false, skip!");
        return;
    }
    #if 0 //不需要识别，直接写入AIUI
    audioPadding = paddingIatAudio((char*)audioData, audioLen, userData);
    if (!audioPadding)
    {
        //已经识别到结果
        usDta->wakeUpState = FALSE;
    }
    #endif
    /* 发送音频数据到AIUI, wwm + 2019/10/11 */
    //LOG(ERROR, "AIUI write audio data, len: %u\n", audioLen);
    g_AIUIImpl.WriteAudioData((char *)audioData, audioLen);
}

BOOL CCAE::MSP_Login(const char *loginParams)
{
    int ret;

    ret = MSPLogin(NULL, NULL, loginParams);
    if(ret != MSP_SUCCESS)
    {
        LOG(ERROR, "MSPLogin failed\n");
        MSPLogout();
        return FALSE;
    }

    return TRUE;
}

BOOL CCAE::AlsaInit()
{
    if (NULL == m_lpCaptureHnd)
    {
        alsa_open(&m_lpCaptureHnd, m_nChn, m_nRate, m_tFormat);
        if (snd_pcm_prepare(m_lpCaptureHnd) < 0)
        {
            LOG(ERROR, "Cannot prepare audio interface for use!\n");
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CCAE::Init()
{
    #if 0
    if (FALSE == AlsaInit())
    {
        LOG(ERROR, "AlsaInit failed !\n");
        return FALSE;
    }
    LOG(INFO, "AlsaInit ok.\n");
    #endif

    //init fun
    if(initCaeFuncs() != MSP_SUCCESS)
    {
        LOG(ERROR, "initCaeFuncs() failed\n");
        return FALSE;
    }
    LOG(INFO, "initCaeFuncs ok.\n");

    int ret = api_cae_new(&m_lpCaeHnd, kIwvResPath, CAEIvwCb, NULL, CAEAudioCb, NULL, &m_UserData);
    if(ret != MSP_SUCCESS)
    {
        LOG(ERROR, "CAENew ... failed!\n");
        return FALSE;
    }
    LOG(INFO, "api_cae_new ok.\n");
#if 0
    // msp_login
    if (!MSP_Login(kMSCLoginParams))
    {
        LOG(ERROR, "MSP_Login failed!\n");
        return FALSE;
    }
    LOG(INFO, "MSP_Login ok.\n");
#endif
    AudioSDK_RegisterStateChangeCallback(&CCAE::StateChange, this);

    return CreateThread();
}

BOOL CCAE::CleanUp()
{
    AudioSDK_UnregisterStateChangeCallback(&CCAE::StateChange, this);
    return DestroyThread(TRUE);
}

#if 0
void CCAE::ThreadProc()
{
    int capture_len = 1024;
    int err;
    size_t BUFSIZE = (int)(capture_len * snd_pcm_format_width(m_tFormat) / 8 * m_nChn);
    char buffer[BUFSIZE] = {0};
    int frame_len;
    size_t buffer_len;
    char cae_audio_data[BUFSIZE * 2];

    while(m_bLoop)
    {
        frame_len = snd_pcm_readi(m_lpCaptureHnd, buffer, capture_len);
        if (frame_len > 0)
        {
            if(m_UserData.wakeUping)
            {
                LOG(INFO, "---caeAudioWrite waiting for wakeup word finished---\n");
                continue;
            }

            buffer_len = (int)(frame_len * snd_pcm_format_width(m_tFormat) / 8 * m_nChn);

            preProcessBuffer(buffer, cae_audio_data, buffer_len);

            err = api_cae_audio_write(m_lpCaeHnd, cae_audio_data, buffer_len * 2);

            if(err != 0)
            {
                LOG(ERROR, "CAEAudioWrite with errCode:%d\n", err);
            }

        }
    }

    LOG(INFO, "CAE thread exit.\n");
}
#else
void CCAE::ThreadProc()
{
    int capture_len = 1024;
    int err;
    size_t BUFSIZE = (int)(capture_len * snd_pcm_format_width(m_tFormat) / 8 * m_nChn);
    char buffer[BUFSIZE] = {0};
    int frame_len;
    size_t buffer_len;
    char cae_audio_data[BUFSIZE * 2];

    while(m_bLoop)
    {
        frame_len = g_AlsaCapture.readStream(buffer, capture_len);
        if (frame_len > 0)
        {
            if(m_UserData.wakeUping)
            {
                LOG(INFO, "---caeAudioWrite waiting for wakeup word finished---\n");
                continue;
            }

            buffer_len = (int)(frame_len * snd_pcm_format_width(m_tFormat) / 8 * m_nChn);

            preProcessBuffer(buffer, cae_audio_data, buffer_len);

            err = api_cae_audio_write(m_lpCaeHnd, cae_audio_data, buffer_len * 2);

            if(err != 0)
            {
                LOG(ERROR, "CAEAudioWrite with errCode:%d\n", err);
            }

        }
    }

    LOG(INFO, "CAE thread exit.\n");
}

#endif
int CCAE::StateChange(int state, void * priv)
{
    if(RETURN_OK == ((CCAE *)priv)->OnStateChange(state))
    {
        return 0;
    }
    return -1;
}

RETURN_TYPE_E CCAE::OnStateChange(int state)
{
    if (m_UserData.wakeUpState && (AIUIConstant::STATE_WORKING != state))
    {
        LOG(INFO, "Clean cae wakeUpState!\n");
        m_UserData.wakeUpState = FALSE;
    }

    return RETURN_OK;
}

int CCAE::preProcessBuffer(void *data, void *out, int bytes)
{
    int channels = 8;
    int is24bitMode = 0;
    int i = 0, j = 0;

    if(!is24bitMode)
    {
        for(i = 0; i < bytes / 2;)
        {
            for(j = 0; j < channels; j++)
            {
                int tmp = 0;
                short tmp_data = (*((short *)data + i + j));
                tmp = ((tmp_data) << 16 | ((j + 1) << 8)) & 0xffffff00;
                *((int *)out + i + j) = tmp;
            }
            i += channels;
        }
    }
    else
    {
        for(i = 0; i < bytes / 4;)
        {
            for(j = 0; j < channels; j++)
            {
                int tmp = 0;
                int tmp_data = (*((int *)data + i + j)) << 1;
                tmp = ((((tmp_data & 0xfffff000) >> 12) << 12) | ((j+1) << 8)) & 0xffffff00;
                *((int *)out + i + j) = tmp;
            }

            i += channels;
        }
    }

    return 0;
}

void CCAE::alsa_open(snd_pcm_t** capture_handle, int channels,
               uint32_t rate, snd_pcm_format_t format)
{
    snd_pcm_hw_params_t *hw_params;
    int err;

    if ((err = snd_pcm_open(capture_handle, "6mic_loopback", /*"default"*/
                            SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        fprintf(stderr, "cannot open audio device %s\n",
                snd_strerror(err));
        exit(1);
    }

    if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if((err = snd_pcm_hw_params_any(*capture_handle, hw_params)) < 0)
    {
        fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_access(*capture_handle, hw_params,
                                            SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        fprintf(stderr, "cannot set access type (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_format(*capture_handle,
                                            hw_params, format)) < 0)
    {
        fprintf(stderr, "cannot set sample format (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_channels(*capture_handle,
                                              hw_params, channels)) < 0)
    {
        fprintf(stderr, "cannot set channel count (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near(*capture_handle,
                                               hw_params, &rate, 0)) < 0)
    {
        fprintf(stderr, "cannot set sample rate (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    snd_pcm_uframes_t period_frames = 512;
    if((err = snd_pcm_hw_params_set_period_size_near(*capture_handle,
              hw_params, &period_frames, 0)) < 0)
    {
        fprintf(stderr, "cannot set sample rate (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if((err = snd_pcm_hw_params(*capture_handle, hw_params)) < 0)
    {
        fprintf(stderr, "cannot set parameters (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    snd_pcm_hw_params_free(hw_params);
}
