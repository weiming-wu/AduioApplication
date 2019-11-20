#ifndef __PCM_FILE_PLAYER_IMPL_H__
#define __PCM_FILE_PLAYER_IMPL_H__
#include <string>
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

#endif //defined _RESAMPLE

class CPcmFilePlayerImpl: public CThread
{
public:
    CPcmFilePlayerImpl(const char *filePath);
    virtual ~CPcmFilePlayerImpl();
    BOOL Init();
    BOOL CleanUp();
    virtual void ThreadProc();
    void PlayStream(uint32 channels,const void* buffer, uint32 buff_size);
    void StartPlay();
private:
    FILE *m_lpFile;
    pthread_mutex_t m_Mutex;
    pthread_cond_t  m_Cond;

#if defined _RESAMPLE
    SwrContext *m_lpSwrCtx;
    uint8_t *m_lpPcmBuffer;
    void Resample(const void *buffer, uint32 buffer_size);
#endif
};


#endif