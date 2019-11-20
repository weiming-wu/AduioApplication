#ifndef __PCM_TTS_PLAYER_IMPL_H__
#define __PCM_TTS_PLAYER_IMPL_H__

#include "Thread.h"

#if defined _RESAMPLE
#ifdef __cplusplus
extern "C"
{
#endif

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include<libswresample/swresample.h>

#ifdef __cplusplus
}
#endif
#endif
class CPcmTtsPlayerImpl: public CThread
{
    PATTERN_SINGLETON_DECLARE(CPcmTtsPlayerImpl);
public:
    //void registerListener(std::shared_ptr<TtsPlayerListener> listener) override;
    BOOL Init();
    BOOL CleanUp();
    virtual void ThreadProc();

    void TtsPlay();

    void PushData(const char* data, uint32 len);

    void PlayStream(uint32 channels,const void* buffer, uint32 buffer_size);

    void TtsEnd();

    void TtsStop();

protected:
    CPcmTtsPlayerImpl();
    ~CPcmTtsPlayerImpl();

private:
    //std::vector<std::shared_ptr<TtsPlayerListener> > m_listeners;
    char* m_lpPcmBuffer;
    uint32 m_nCurrentLen;
    uint32 m_TotalLen;
    pthread_mutex_t m_PlayMutex;
    pthread_mutex_t m_MemMutex;
    pthread_cond_t m_PlayCond;
    //StreamPool m_streamPool;
    BOOL m_bPushingFlag;
    BOOL m_bStopFlag;
    //BOOL m_bIsFirstPacket;

#if defined _RESAMPLE
    SwrContext *m_lpSwrCtx;
    uint8_t *output;
    void Resample(const void *buffer, uint32 buffer_size);
#endif
};

#define g_PcmTtsPlayerImpl    (*CPcmTtsPlayerImpl::instance())
#endif
