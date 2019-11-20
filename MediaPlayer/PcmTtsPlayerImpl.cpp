#include "PcmTtsPlayerImpl.h"
#include "AlsaPlayback.h"
#include "SysLog.h"
#define PCM_BUFFER_LEN 256000
#define WAIT_TIME_INTERVAL_MS 100000
#define PLAY_INCREMENT_LEN 2048

PATTERN_SINGLETON_IMPLEMENT(CPcmTtsPlayerImpl);

CPcmTtsPlayerImpl::CPcmTtsPlayerImpl()
    : CThread(FALSE, 0, 0, "PcmTtsPlayerImpl")
    , m_lpPcmBuffer(NULL)
    , m_nCurrentLen(0)
    , m_TotalLen(PLAY_INCREMENT_LEN)
    , m_bPushingFlag(FALSE)
    , m_bStopFlag(FALSE)
#if defined _RESAMPLE
    , m_lpSwrCtx(NULL)
    , output(NULL)
#endif
{
}

CPcmTtsPlayerImpl::~CPcmTtsPlayerImpl()
{
    m_nCurrentLen = 0;
    m_TotalLen = 0;
    m_bPushingFlag = 0;
    m_bPushingFlag = FALSE;
    m_bStopFlag = FALSE;
}

BOOL CPcmTtsPlayerImpl::Init()
{
    m_lpPcmBuffer = (char *)malloc(PLAY_INCREMENT_LEN);
    if (NULL == m_lpPcmBuffer)
    {
        LOG(ERROR, "Not enough memory !\n");
        return FALSE;
    }
    pthread_mutex_init(&m_PlayMutex, NULL);
    pthread_mutex_init(&m_MemMutex, NULL);
    pthread_cond_init(&m_PlayCond, NULL);

    return CreateThread();
}

BOOL CPcmTtsPlayerImpl::CleanUp()
{
    if (NULL != m_lpPcmBuffer)
    {
        free(m_lpPcmBuffer);
        m_lpPcmBuffer = NULL;
    }

#if defined _RESAMPLE
    if (m_lpSwrCtx)
    {
        swr_free(&m_lpSwrCtx);
        m_lpSwrCtx = NULL;
    }
    if (output)
    {
        av_freep(&output);
    }
#endif

    pthread_mutex_destroy(&m_PlayMutex);
    pthread_mutex_destroy(&m_MemMutex);
    pthread_cond_destroy(&m_PlayCond);

    return DestroyThread(TRUE);
}

void CPcmTtsPlayerImpl::PushData(const char* data, uint32 len)
{
    if (!m_bPushingFlag)
    {
        return;
    }

    pthread_mutex_lock(&m_MemMutex);
    if (m_nCurrentLen + len > m_TotalLen)
    {
        int incre_len = PCM_BUFFER_LEN / 4;
        char* tmp = (char*)malloc(m_TotalLen + incre_len);
        memcpy(tmp, m_lpPcmBuffer, m_nCurrentLen);
        //pthread_mutex_lock(&m_PlayMutex);
        free(m_lpPcmBuffer);
        m_lpPcmBuffer = tmp;
        //pthread_mutex_unlock(&m_PlayMutex);
        m_TotalLen += incre_len;
    }

    memcpy(m_lpPcmBuffer + m_nCurrentLen, data, len);
    m_nCurrentLen += len;
    pthread_mutex_unlock(&m_MemMutex);
    LOG(INFO, "PushData len: %u\n", len);
}

#if defined _RESAMPLE
void CPcmTtsPlayerImpl::Resample(const void *buffer, uint32 buff_size)
{
    int in_nb_samples = PCM_NB_SAMPLES;
    int in_sample_rates = PCM_SAMPLE_RATES;
    int out_nb_samples = in_nb_samples * SND_PLAYBACK_SAMPLE_RATES / in_sample_rates + 256;

    if(NULL == m_lpSwrCtx)
    {
        if(m_lpSwrCtx != NULL)
            swr_free(&m_lpSwrCtx);
        LOG(INFO, "AV_CH_LAYOUT_STEREO=%d, AV_SAMPLE_FMT_S16=%d, freq=44100\n", AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16);
        //printf("frame: channnels=%d, default_layout=%d, format=%d, sample_rate=%d", frame->channels,av_get_default_channel_layout(frame->channels), frame->format, frame->sample_rate);

        if (NULL == output)
        {
            av_samples_alloc(&output, NULL, SND_PLAYBACK_CHANNELS, out_nb_samples, AV_SAMPLE_FMT_S16, 0);
        }
        m_lpSwrCtx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, SND_PLAYBACK_SAMPLE_RATES,
                                        AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, PCM_SAMPLE_RATES, 0, NULL);
        if(m_lpSwrCtx == NULL)
        {
            LOG(ERROR, "m_lpSwrCtx == NULL");
        }
        swr_init(m_lpSwrCtx);
    }

    int convert_size = swr_convert(m_lpSwrCtx, &output, out_nb_samples,
                                   (const uint8_t **)&buffer,
                                   in_nb_samples);
    int out_size = av_samples_get_buffer_size(NULL, SND_PLAYBACK_CHANNELS, convert_size, AV_SAMPLE_FMT_S16, 1);
    g_AlsaPlayback.writeStream(output, out_size);

}
#endif
void CPcmTtsPlayerImpl::PlayStream(uint32 channels, const void* buffer, uint32 buff_size)
{
    /*
    if (m_isFirstPacket)
    {
        executePlaybackStarted();
        m_isFirstPacket = false;
    }
    */
    #if defined _RESAMPLE
    //LOG(INFO, "Playstream resample !!!!!!\n");
    Resample(buffer, buff_size);
    #else
    g_AlsaPlayback.writeStream(buffer, buff_size, channels);
    #endif
}

void CPcmTtsPlayerImpl::TtsPlay()
{
    g_AlsaPlayback.alsaPrepare();
    pthread_mutex_lock(&m_PlayMutex);
    //m_isFirstPacket = true;
    m_bStopFlag = FALSE;
    m_bPushingFlag = TRUE;
    pthread_cond_signal(&m_PlayCond);
    pthread_mutex_unlock(&m_PlayMutex);
}

void CPcmTtsPlayerImpl::TtsEnd()
{
    m_bPushingFlag = FALSE;
}

void CPcmTtsPlayerImpl::TtsStop()
{
    //g_AlsaPlayback.aslaAbort();
    //g_AlsaPlayback.alsaClear();
    m_bPushingFlag = FALSE;
    m_bStopFlag = TRUE;
    //executePlaybackStopped();
}

void CPcmTtsPlayerImpl::ThreadProc()
{
    while (m_bLoop)
    {
        LOG(INFO, "PcmTtsPlayerImpl thread start...\n");
        pthread_mutex_lock(&m_PlayMutex);
        pthread_cond_wait(&m_PlayCond, &m_PlayMutex);
        g_AlsaPlayback.alsaPrepare();

        if (!m_bLoop)
        {
            break;
        }

        int playLen = 0;

        while (!m_bStopFlag)
        {
            if (playLen + PLAY_INCREMENT_LEN < (int)m_nCurrentLen)
            {
                pthread_mutex_lock(&m_MemMutex);
                PlayStream(1, m_lpPcmBuffer + playLen, PLAY_INCREMENT_LEN);
                pthread_mutex_unlock(&m_MemMutex);
                playLen += PLAY_INCREMENT_LEN;
                LOG(INFO, "loop play stream, playLen: %d\n", playLen);
            }
            else if (m_bPushingFlag)
            {
                usleep(WAIT_TIME_INTERVAL_MS);
            }
            else
            {
                pthread_mutex_lock(&m_MemMutex);
                PlayStream(1, m_lpPcmBuffer + playLen, m_nCurrentLen - playLen);
                pthread_mutex_unlock(&m_MemMutex);
                break;
            }
        }
//        LOG(INFO, "exit play stream !!!\n");
        if (!m_bStopFlag)
        {
            TtsStop();
            //player->executePlaybackStopped();
        }
        pthread_mutex_lock(&m_MemMutex);
        memset(m_lpPcmBuffer, 0, m_TotalLen);
        m_nCurrentLen = 0;
        pthread_mutex_unlock(&m_MemMutex);
        pthread_mutex_unlock(&m_PlayMutex);
    }

}

