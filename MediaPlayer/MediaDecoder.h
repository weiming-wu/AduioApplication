#ifndef __MEDIA_DECODER_H__
#define __MEDIA_DECODER_H__
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
#include <list>
#include "BaseFunction.h"
#include "Thread.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE1 192000

typedef void (*DEC_CALLBACK_FUN)(const void *data, uint32 dataLen, void *context);
typedef struct{
	//int videoindex;
	int sndindex;
	AVFormatContext *pFormatCtx;
	AVCodecContext *sndCodecCtx;
	AVCodec *sndCodec;
	SwrContext *swr_ctx;
	//snd_pcm_t *pcm;
	DECLARE_ALIGNED(16,uint8_t,audio_buf) [AVCODEC_MAX_AUDIO_FRAME_SIZE1 * 4];
}AudioState_t;

typedef enum
{
    STOP_DECODE     = 0,
    START_DECODE    = 1,
    PAUSE_DECODE    = 2,
    RESUME_DECODE   = 3,
}EN_DECODE_STATE;

class CMediaDecoder: public CThread
{
public:
    CMediaDecoder(const char *filePath);
    virtual ~CMediaDecoder();
    virtual void ThreadProc();
    BOOL Init();
    BOOL CleanUp();
    void RegisterDecCallback(DEC_CALLBACK_FUN fun, void *context);
    void UnregisterDecCallback(DEC_CALLBACK_FUN fun, void *context);
    void SetDecodeState(EN_DECODE_STATE state);
    EN_DECODE_STATE GetDecodeState()const;
    void Start();
    void Pause();
    void Resume();
    void Stop();
protected:
    BOOL InitCodec();
    void DecAudioFrameNotify(const void * data, uint32 dataLen);
private:
    char m_cFileName[128];
    std::list<CCallbackInfo<DEC_CALLBACK_FUN> > m_listDecoderCb;
    //std::map<DEC_CALLBACK_FUN, void *> m_mapDecCb;
    AudioState_t m_tAudiaState;
    EN_DECODE_STATE m_enDecState;
    pthread_mutex_t    m_DecMutex;
    pthread_cond_t     m_DecCond;
};


#endif
