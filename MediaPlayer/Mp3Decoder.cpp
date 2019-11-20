/******************************************************************************

                  版权所有 (C), 2018-2099, 恒大集团

 ******************************************************************************
  文 件 名   : Mp3Decoder.cpp
  版 本 号   : 初稿
  作    者   : wuweiming
  生成日期   : 2019年10月22日
  最近修改   :
  功能描述   : 本文件仅用于测试mp3解码为pcm功能

  修改历史   :
  1.日    期   : 2019年10月22日
    作    者   : wuweiming
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#include "Mp3Decoder.h"
#include <sys/stat.h>
#include "SysLog.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

DECLARE_ALIGNED(16,uint8_t,audio_buf) [AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];

Mp3Decoder::Mp3Decoder(const char *filePath)
	: errMsg(NULL)
	, fmt_ctx(NULL)
	, audio_stream_idx(0)
	, audio_stream(NULL)
	, audio_dec_ctx(NULL)
	, audio_frame_count(0)
	, putBuffer(NULL)
	, putBufferData(NULL)
	, n_channels(0)
	, fmt(NULL)
	, sample_rate(0)
{
    int len = strlen(filePath);
    src_filename = new char[sizeof(char)*(len+100)];
    snprintf(src_filename,len+100,"%s",filePath);
}

int Mp3Decoder::checkFileOk() {
    struct stat buf;
    int ret = stat(src_filename, &buf);
    if (buf.st_mode |S_IFMT) {
        return ret;
    }
    return -1;
}

Mp3Decoder::~Mp3Decoder() {
    if (src_filename) {
        delete src_filename;
        src_filename = NULL;
    }
    if (errMsg) {
        delete  errMsg;
        errMsg = NULL;
    }
}

void Mp3Decoder::setErrorMsg(const char *msg, ...) {
    if (!errMsg) {
        errMsg = new char[256];
    }
}

void Mp3Decoder::start() {
    int ret =0, got_frame;
    AVFrame *frame = NULL;
    AVPacket pkt;

    av_dump_format(fmt_ctx,0,src_filename,0);

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        setErrorMsg("Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    while (av_read_frame(fmt_ctx,&pkt)>=0) {
        AVPacket orig_pkt = pkt;
        do {
            ret = decode_packet(&pkt,frame,&got_frame,0);
            pkt.data += ret;
            pkt.size -= ret;
        } while (pkt.size>0);
        av_packet_unref(&orig_pkt);
    }

    pkt.data = NULL;
    pkt.size = 0;
    do {
        ret = decode_packet(&pkt,frame,&got_frame,0);
    } while (got_frame);

end:
    av_frame_free(&frame);
    avcodec_free_context(&audio_dec_ctx);
}

int Mp3Decoder::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx,
                                   AVFormatContext *fmt_ctx, enum AVMediaType type) {
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *ops = NULL;

    //find the stream
    ret = av_find_best_stream(fmt_ctx,type,-1,-1,NULL,0);
    if (ret<0) {
        LOG(ERROR, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        LOG(ERROR, "Could not find %s stream in input file '%s'\n",
                    av_get_media_type_string(type), src_filename);
        return ret;
    } else{
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            LOG(ERROR, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            LOG(ERROR, "Failed to find %s codec\n",
                        av_get_media_type_string(type));
            return AVERROR(EINVAL);

        }

        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) { // alloc falis
            LOG(ERROR, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            LOG(ERROR, "Failed to allocate the %s codec context\n",
                         av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        if ((ret = avcodec_parameters_to_context(*dec_ctx,st->codecpar))<0){
            LOG(ERROR, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            LOG(ERROR, "Failed to copy %s codec parameters to decoder context\n",
                        av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders, with or without reference counting */
        av_dict_set(&ops,"refcounted_frames","0",0);
        if ((ret = avcodec_open2(*dec_ctx,dec,&ops))<0){
            LOG(ERROR, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            LOG(ERROR, "Failed to open %s codec\n",
                        av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

static SwrContext *swr_ctx = NULL;

int Mp3Decoder::decode_packet(AVPacket *pPkt,AVFrame *frame,int *got_frame, int cached) {
    uint8_t *out[] = {audio_buf};

    int ret =0;
    int decoded = pPkt->size;
    if (pPkt->stream_index!=audio_stream_idx) {
        pPkt->size = 0;
        return ret;
    }

    *got_frame = 0;

    ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, pPkt);
    if (ret<0){
        //LOG(ERROR, "Error decoding audio frame (%s)\n", av_err2str(ret));
        //LOG(ERROR, "Error decoding audio frame (%s)\n", av_err2str(ret));
        return ret;
    }
    /* Some audio decoders decode only part of the packet, and have to be
      * called again with the remainder of the packet data.
      * Sample: fate-suite/lossless-audio/luckynight-partial.shn
      * Also, some decoders might over-read the packet. */
    decoded = FFMIN(ret,pPkt->size);

    if (*got_frame) {
        /*
        size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(
                (AVSampleFormat) frame->format);
        */
				/*
        LOG(INFO, "audio_frame%s n:%d nb_samples:%d pts:%s\n",
               cached ? "(cached)" : "",
               audio_frame_count++, frame->nb_samples,
               av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));
			   */

        /* Write the raw audio data samples of the first plane. This works
         * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
         * most audio decoders output planar audio, which uses a separate
         * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
         * In other words, this code will write only the first audio channel
         * in these cases.
         * You should use libswresample or libavfilter to convert the frame
         * to packed data. */
        if(NULL == swr_ctx)
        {
            enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
            if(swr_ctx != NULL)
                swr_free(&swr_ctx);
            printf("AV_CH_LAYOUT_STEREO=%d, AV_SAMPLE_FMT_S16=%d, freq=44100\n", AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16);
            printf("frame: channnels=%d, format=%d, sample_rate=%d\n",
                    frame->channels,
                    frame->format,
                    frame->sample_rate);

            swr_ctx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, out_sample_fmt/*AV_SAMPLE_FMT_S16*/, audio_dec_ctx->sample_rate,
                                        av_get_default_channel_layout(frame->channels), audio_dec_ctx->sample_fmt/*frame->format*/, frame->sample_rate, 0, NULL);
            if(swr_ctx == NULL)
            {
                LOG(ERROR, "swr_ctx == NULL");
            }
            swr_init(swr_ctx);
        }

        swr_convert(swr_ctx, out, audio_dec_ctx->sample_rate,(const uint8_t **)frame->extended_data, frame->nb_samples);
        int data_size = av_samples_get_buffer_size(NULL, audio_dec_ctx->channels, frame->nb_samples, audio_dec_ctx->sample_fmt, 1);

//        fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
        if (putBuffer) {
            //LOG(ERROR, "data size %s",frame->extended_data);
            //putBuffer(frame->extended_data[0],1,unpadded_linesize,putBufferData);
            //putBuffer((const void *)frame->extended_data[0], unpadded_linesize, putBufferData);
            putBuffer((const void *)audio_buf, data_size, NULL);
        }
    }

    /* If we use frame reference counting, we own the data and need
    * to de-reference it when we don't use it anymore */
    if (*got_frame & 0)
        av_frame_unref(frame);

    return decoded;
}

int Mp3Decoder::prepare() {
    int ret=0;

    av_register_all();
    avformat_network_init();
    fmt_ctx = avformat_alloc_context();

    //1 打开文件获取fmt_ctx(用于解封装)
    if(avformat_open_input(&fmt_ctx,src_filename,NULL,NULL)<0){
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        LOG(ERROR, "Could not open source file %s\n", src_filename);
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx,NULL)<0) {
        fprintf(stderr, "Could not find stream information\n");
        LOG(ERROR, "Could not find stream information\n");
        ret = -1;
        goto close_fmt;
    }

    //找到audio stream 并且获取相应的codec(解码)
    if(open_codec_context(&audio_stream_idx,&audio_dec_ctx,fmt_ctx,AVMEDIA_TYPE_AUDIO)>=0) {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
    } else {
        fprintf(stderr, "Could not find stream information\n");
        LOG(ERROR, "Could not find stream information\n");
        ret = -1;
        goto close_fmt;
    }

    if (audio_stream) {
        enum AVSampleFormat sfmt = audio_dec_ctx->sample_fmt;
        n_channels = audio_dec_ctx->channels;
        sample_rate = audio_dec_ctx->sample_rate;

        if (av_sample_fmt_is_planar(sfmt)) {
            const char *packed = av_get_sample_fmt_name(sfmt);
            LOG(WARNING, "Warning: the sample format the decoder produced is planar "
                           "(%s). This example will output the first channel only.\n",
                   packed ? packed : "?");
            sfmt = av_get_packed_sample_fmt(sfmt);
            n_channels = 1;
        }

        if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
            goto close_fmt;
        else {
            goto end;
        }
    } else {
        ret = -2;
    }

close_fmt:
    avformat_close_input(&fmt_ctx);

end:
    return ret;
}


int Mp3Decoder::get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt) {
    unsigned int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
            {AV_SAMPLE_FMT_U8,  "u8",    "u8"},
            {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
            {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
            {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
            {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }
    LOG(ERROR, "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}
