#include <string.h>
#include "MediaDecoder.h"
#include "SysLog.h"
#include "AlsaPlayback.h"

CMediaDecoder::CMediaDecoder(const char *filePath)
    : CThread(FALSE, 0, 0, "MediaDecoder")
    , m_enDecState(STOP_DECODE)
{
    if (NULL == filePath)
    {
        m_cFileName[0] = '\0';
    }
    else
    {
        strncpy(m_cFileName, filePath, sizeof(m_cFileName));
    }
}

CMediaDecoder::~CMediaDecoder()
{
    LOG(INFO, "~CMediaDecoder\n");
}

BOOL CMediaDecoder::Init()
{
    BOOL bRet = FALSE;
    pthread_mutex_init(&m_DecMutex, NULL);
    pthread_cond_init(&m_DecCond, NULL);
    if (FALSE == InitCodec())
    {
        LOG(ERROR ,"InitCodec failed !\n");
        goto FAILED;
    }
    LOG(INFO, "InitCodec ok.\n");
    return CreateThread();
FAILED:
    CleanUp();
    return bRet;
}

BOOL CMediaDecoder::InitCodec()
{
    m_tAudiaState.sndindex = -1;
    if('\0' == m_cFileName[0])
    {
        LOG(ERROR, "input file is NULL");
        return FALSE;
    }
    avcodec_register_all();
    av_register_all();
    avformat_network_init();

    m_tAudiaState.pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&m_tAudiaState.pFormatCtx, m_cFileName, NULL, NULL)!=0)
    {
        LOG(ERROR, "avformat_open_input failed !\n");
        return FALSE;
    }

    if(avformat_find_stream_info(m_tAudiaState.pFormatCtx, NULL)<0)
    {
        LOG(ERROR, "avformat_find_stream_info failed !\n");
        return FALSE;
    }

    av_dump_format(m_tAudiaState.pFormatCtx, 0, 0, 0);
    //m_tAudiaState.videoindex = av_find_best_stream(is->pFormatCtx, AVMEDIA_TYPE_VIDEO, is->videoindex, -1, NULL, 0);
    m_tAudiaState.sndindex = av_find_best_stream(m_tAudiaState.pFormatCtx, AVMEDIA_TYPE_AUDIO,m_tAudiaState.sndindex, -1/*m_tAudiaState.videoindex*/, NULL, 0);

    if(m_tAudiaState.sndindex != -1)
    {
        #if 0
        m_tAudiaState.sndCodecCtx = m_tAudiaState.pFormatCtx->streams[m_tAudiaState.sndindex]->codec;
        m_tAudiaState.sndCodec = avcodec_find_decoder(m_tAudiaState.sndCodecCtx->codec_id);
        if(m_tAudiaState.sndCodec == NULL)
        {
            LOG(ERROR, "Codec not found\n");
            return FALSE;
        }
        #else
        m_tAudiaState.sndCodec = avcodec_find_decoder(m_tAudiaState.pFormatCtx->streams[m_tAudiaState.sndindex]->codecpar->codec_id);
        m_tAudiaState.sndCodecCtx = avcodec_alloc_context3(NULL);
        avcodec_parameters_to_context(m_tAudiaState.sndCodecCtx, m_tAudiaState.pFormatCtx->streams[m_tAudiaState.sndindex]->codecpar);
        #endif
        if(avcodec_open2(m_tAudiaState.sndCodecCtx, m_tAudiaState.sndCodec, NULL) < 0)
        {
            LOG(ERROR, "avcodec open failed !\n");
            return FALSE;
        }
    }
    return TRUE;
}

void CMediaDecoder::Start()
{
    pthread_mutex_lock(&m_DecMutex);
    m_enDecState = START_DECODE;
    pthread_cond_signal(&m_DecCond);
    pthread_mutex_unlock(&m_DecMutex);
    LOG(INFO, "Start decode. \n");
}

void CMediaDecoder::Pause()
{
    pthread_mutex_lock(&m_DecMutex);
    m_enDecState = PAUSE_DECODE;
    pthread_cond_signal(&m_DecCond);
    pthread_mutex_unlock(&m_DecMutex);
    LOG(INFO, "Pause decode. \n");
}

void CMediaDecoder::Resume()
{
    pthread_mutex_lock(&m_DecMutex);
    m_enDecState = RESUME_DECODE;
    pthread_cond_signal(&m_DecCond);
    pthread_mutex_unlock(&m_DecMutex);
    LOG(INFO, "Resume decode. \n");
}

void CMediaDecoder::Stop()
{
    pthread_mutex_lock(&m_DecMutex);
    m_enDecState = STOP_DECODE;
    pthread_cond_signal(&m_DecCond);
    pthread_mutex_unlock(&m_DecMutex);
    LOG(INFO, "Stop decode. \n");
}

BOOL CMediaDecoder::CleanUp()
{
    pthread_mutex_destroy(&m_DecMutex);
    pthread_cond_destroy(&m_DecCond);

    if(m_tAudiaState.swr_ctx != NULL)
    {
        swr_free(&m_tAudiaState.swr_ctx);
        m_tAudiaState.swr_ctx = NULL;
    }
    if (NULL != m_tAudiaState.sndCodecCtx)
    {
        avcodec_close(m_tAudiaState.sndCodecCtx);
    }
    if (NULL != m_tAudiaState.pFormatCtx)
    {
        avformat_close_input(&m_tAudiaState.pFormatCtx);
    }

    avformat_network_deinit();
    LOG(INFO, "Media decoder clean up exit.\n");
    return TRUE;
}

void CMediaDecoder::RegisterDecCallback(DEC_CALLBACK_FUN fun, void *context)
{
    #if 1
    CCallbackInfo<DEC_CALLBACK_FUN> listener;
    listener.cb_fun = fun;
    listener.context = context;
    m_listDecoderCb.push_back(listener);
    #else
    if (NULL != fun)
    {
        m_mapDecCb.insert(std::pair<DEC_CALLBACK_FUN, void *>(fun, context));
    }
    #endif
}

void CMediaDecoder::UnregisterDecCallback(DEC_CALLBACK_FUN fun, void *context)
{
    #if 1
    for(std::list<CCallbackInfo<DEC_CALLBACK_FUN> >::iterator it=m_listDecoderCb.begin();
        it!=m_listDecoderCb.end(); it++)
    {
        if ((it->cb_fun == fun) && (it->context == context))
        {
            it->cb_fun = NULL;
            it->context = NULL;
            m_listDecoderCb.erase(it);
        }
    }
    #else
    for (std::map<DEC_CALLBACK_FUN, void *>::iterator it = m_mapDecCb.begin();
        it != m_mapDecCb.end();)
    {
        if ( (it->first == fun) && (it->second == context) )
        {
            it->first = NULL;
            it->second = NULL;
            it = m_mapDecCb.erase(it);  // erase的返回值是指向被删除元素的后继元素的迭代器
        }
        else
        {
            ++it;
        }
    }

    #endif
}

void CMediaDecoder::SetDecodeState(EN_DECODE_STATE state)
{
    pthread_mutex_lock(&m_DecMutex);
    m_enDecState = state;
    pthread_cond_signal(&m_DecCond);
    pthread_mutex_unlock(&m_DecMutex);
}

EN_DECODE_STATE CMediaDecoder::GetDecodeState()const
{
    return m_enDecState;
}

void CMediaDecoder::DecAudioFrameNotify(const void * data, uint32 dataLen)
{
    for(std::list<CCallbackInfo<DEC_CALLBACK_FUN> >::iterator it=m_listDecoderCb.begin();
        it!=m_listDecoderCb.end(); it++)
    {
        if (NULL != it->cb_fun)
        {
            it->cb_fun(data, dataLen, it->context);
        }
    }
}

void CMediaDecoder::ThreadProc()
{
    int got_frame = 0;
    AVPacket *packet = (AVPacket *)av_mallocz(sizeof(AVPacket));
    AVFrame *frame = (AVFrame *)av_frame_alloc();
    //uint8_t *out[] = { m_tAudiaState.audio_buf };
    int out_nb_samples = 0;
    uint8_t *output = NULL;

    while(m_bLoop)
    {
        LOG(INFO, "Media decoder thread start...\n");
        pthread_mutex_lock(&m_DecMutex);
        while((STOP_DECODE == m_enDecState) || (PAUSE_DECODE == m_enDecState))
        {
            pthread_cond_wait(&m_DecCond, &m_DecMutex);
        }
        if (STOP_DECODE == m_enDecState)
        {
            pthread_mutex_unlock(&m_DecMutex);
            break;
        }
        pthread_mutex_unlock(&m_DecMutex);
        while( ((START_DECODE == m_enDecState) || (RESUME_DECODE == m_enDecState))
                && (0 == av_read_frame(m_tAudiaState.pFormatCtx, packet)))
        {
            if (packet->stream_index != m_tAudiaState.sndindex)
                continue;
            if (avcodec_decode_audio4(m_tAudiaState.sndCodecCtx, frame, &got_frame, packet) < 0) //decode data is store in frame
            {
                LOG(ERROR, "Deocde audio error or end of file\n");
                m_bLoop = FALSE; //线程自己退出
                break;
            }

            if(got_frame <= 0) /* No data yet, get more frames */
                continue;
            if(NULL == m_tAudiaState.swr_ctx)
            {
                if(m_tAudiaState.swr_ctx != NULL)
                {
                    swr_free(&m_tAudiaState.swr_ctx);
                }
                //LOG(INFO, "AV_CH_LAYOUT_STEREO=%d, AV_SAMPLE_FMT_S16=%d, freq=44100\n", AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16);

                out_nb_samples = (int64_t)frame->nb_samples * SND_PLAYBACK_SAMPLE_RATES / frame->sample_rate + 256;
                av_samples_alloc(&output, NULL, SND_PLAYBACK_CHANNELS, out_nb_samples, AV_SAMPLE_FMT_S16, 0);
                /*
                LOG(INFO, "sndCodecCtx->sample_rate: %d\n", m_tAudiaState.sndCodecCtx->sample_rate);
                LOG(INFO, "frame->format: %d\n", frame->format);
                LOG(INFO, "frame->sample_rate: %d\n", frame->sample_rate);
                LOG(INFO, "frame->nb_samples: %d, out_nb_samples: %d\n", frame->nb_samples, out_nb_samples);
                */
                m_tAudiaState.swr_ctx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                                           SND_PLAYBACK_SAMPLE_RATES,
                                                           av_get_default_channel_layout(frame->channels),
                                                           (AVSampleFormat)frame->format, frame->sample_rate, 0, NULL);
                if(m_tAudiaState.swr_ctx == NULL)
                {
                    LOG(INFO, "swr_ctx is NULL\n");
                }
                swr_init(m_tAudiaState.swr_ctx);
            }
            int convert_size = swr_convert(m_tAudiaState.swr_ctx, &output, out_nb_samples,
                               (const uint8_t **)frame->extended_data, frame->nb_samples);

            uint32 data_size = av_samples_get_buffer_size(NULL, SND_PLAYBACK_CHANNELS/*out_channels*/,
                                                          convert_size,
                                                          AV_SAMPLE_FMT_S16,
                                                          1);
            //DecAudioFrameNotify(m_tAudiaState.audio_buf, data_size);
            DecAudioFrameNotify(output, data_size);
        }
        if (PAUSE_DECODE != m_enDecState)
        {
            break;
        }
    }
    if (NULL != packet)
    {
        av_free(packet);
    }
    if (NULL != frame)
    {
        av_free(frame);
    }
    if (NULL != output)
    {
        av_freep(&output);
    }
    LOG(INFO, "MediaDecoder thread exit normally.\n");
}

