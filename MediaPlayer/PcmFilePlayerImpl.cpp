#include "PcmFilePlayerImpl.h"
#include "AlsaPlayback.h"
#include "SysLog.h"

CPcmFilePlayerImpl::CPcmFilePlayerImpl(const char *filePath)
    : CThread(FALSE, 0, 0, "PcmFilePlayerImpl")
    , m_lpFile(NULL)
#if defined _RESAMPLE
    , m_lpSwrCtx(NULL)
    , m_lpPcmBuffer(NULL)
#endif
{
    m_lpFile = fopen(filePath, "rb");
    pthread_mutex_init(&m_Mutex, NULL);
    pthread_cond_init(&m_Cond, NULL);
}

CPcmFilePlayerImpl::~CPcmFilePlayerImpl()
{
    if (NULL != m_lpFile)
    {
        fclose(m_lpFile);
    }
#if defined _RESAMPLE
    if (m_lpSwrCtx)
    {
        swr_free(&m_lpSwrCtx);
        m_lpSwrCtx = NULL;
    }
    if (m_lpPcmBuffer)
    {
        av_freep(&m_lpPcmBuffer);
    }
#endif

    pthread_mutex_destroy(&m_Mutex);
    pthread_cond_destroy(&m_Cond);
    DestroyThread(TRUE);
}

BOOL CPcmFilePlayerImpl::Init()
{
    if (NULL == m_lpFile)
    {
        LOG(ERROR, "CPcmFilePlayerImpl init failed, m_lpFile is null!\n");
        return FALSE;
    }
    return CreateThread();
}

BOOL CPcmFilePlayerImpl::CleanUp()
{
    //return DestroyThread(TRUE);
    return TRUE;
}

void CPcmFilePlayerImpl::StartPlay()
{
    pthread_mutex_lock(&m_Mutex);
    pthread_cond_signal(&m_Cond);
    pthread_mutex_unlock(&m_Mutex);
    LOG(INFO, "StartPlay. \n");
}

#if defined _RESAMPLE
void CPcmFilePlayerImpl::Resample(const void *buffer, uint32 buff_size)
{
    int in_nb_samples = PCM_NB_SAMPLES;
    int in_sample_rates = PCM_SAMPLE_RATES;
    int out_nb_samples = in_nb_samples * SND_PLAYBACK_SAMPLE_RATES / in_sample_rates + 256;

    if(NULL == m_lpSwrCtx)
    {
        if(m_lpSwrCtx != NULL)
            swr_free(&m_lpSwrCtx);
        //LOG(INFO, "AV_CH_LAYOUT_STEREO=%d, AV_SAMPLE_FMT_S16=%d, freq=44100\n", AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16);
        //printf("frame: channnels=%d, default_layout=%d, format=%d, sample_rate=%d", frame->channels,av_get_default_channel_layout(frame->channels), frame->format, frame->sample_rate);

        if (NULL == m_lpPcmBuffer)
        {
            av_samples_alloc(&m_lpPcmBuffer, NULL, SND_PLAYBACK_CHANNELS, out_nb_samples, AV_SAMPLE_FMT_S16, 0);
        }
        m_lpSwrCtx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, SND_PLAYBACK_SAMPLE_RATES,
                                        AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, PCM_SAMPLE_RATES, 0, NULL);
        if(m_lpSwrCtx == NULL)
        {
            LOG(ERROR, "m_lpSwrCtx == NULL");
        }
        swr_init(m_lpSwrCtx);
    }

    int convert_size = swr_convert(m_lpSwrCtx, &m_lpPcmBuffer, out_nb_samples,
                                   (const uint8_t **)&buffer,
                                   in_nb_samples);
    int out_size = av_samples_get_buffer_size(NULL, SND_PLAYBACK_CHANNELS, convert_size, AV_SAMPLE_FMT_S16, 1);

    g_AlsaPlayback.writeStream(m_lpPcmBuffer, out_size);
}
#endif


void CPcmFilePlayerImpl::PlayStream(uint32 channels, const void* buffer, uint32 buffer_size)
{
    #if defined _RESAMPLE
    Resample(buffer, buffer_size);
    #else
    g_AlsaPlayback.writeStream(buffer, buffer_size, channels);
    #endif
}

void CPcmFilePlayerImpl::ThreadProc()
{
    while (m_bLoop)
    {
        LOG(INFO, "pcm file player thread start...\n");
        pthread_mutex_lock(&m_Mutex);
        pthread_cond_wait(&m_Cond, &m_Mutex);
        if (!m_bLoop)
        {
            fclose(m_lpFile);
            pthread_mutex_unlock(&m_Mutex);
            break;
        }

        while(!feof(m_lpFile))
        {
            int incLen = PCM_FRAME_SIZE; //每次写1024个字节
            char buffer[PCM_FRAME_SIZE] = {0};
            int size = fread(buffer, 1, incLen, m_lpFile);
            if (size > 0)
            {
                PlayStream(SND_PLAYBACK_CHANNELS, buffer, size);
            }
        }
        fclose(m_lpFile);
        m_lpFile = NULL;
        pthread_mutex_unlock(&m_Mutex);
    }
}
