#ifndef __MP3_DECODER_H__
#define __MP3_DECODER_H__

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
#include "Type.h"
typedef void (*MP3_DECODER_CB)(const void *data, uint32 size, void *priv);
class Mp3Decoder {

public:
    Mp3Decoder(const char *filePath);
    int checkFileOk();
    ~Mp3Decoder();

    void start();

    void setPutBufferImpl(MP3_DECODER_CB fun_cb, void *context) {
        putBuffer = fun_cb;
        putBufferData = context;
    }

    int prepare();
    int getChannelCount() const {
        return n_channels;
    }
    int getSampleRate() const {
        return sample_rate;
    }

    const char* getFmt() const{
        return fmt;
    }

private:
    char *src_filename;
    char *errMsg;
    AVFormatContext *fmt_ctx;
    int audio_stream_idx;
    AVStream *audio_stream;
    AVCodecContext *audio_dec_ctx;
    int audio_frame_count;
    //void (*putBuffer)(const void* buffer,ssize_t size,ssize_t count, const void* data);
    MP3_DECODER_CB putBuffer;
    void* putBufferData;
    int n_channels;
    const char *fmt;
    int sample_rate;

    int get_format_from_sample_fmt(const char **fmt,
                               enum AVSampleFormat sample_fmt);
    void setErrorMsg(const char *msg,...);
    int open_codec_context(int *stream_idx,
                           AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    int decode_packet(AVPacket *pkt,AVFrame *frame, int *got_frame,int cached);
};


#endif //__MP3_DECODER_H__