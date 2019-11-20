#ifndef __MEDIA_PLAYER_H__
#define __MEDIA_PLAYER_H__

#include "MediaDecoder.h"

class CMediaPlayer
{
    PATTERN_SINGLETON_DECLARE(CMediaPlayer);
public:
    BOOL PlayAudioStream(const char *url);
    BOOL PauseAudioStream();
    BOOL ResumeAudioStream();
    BOOL StopAudioStream();
    static void AudioDecFrameReady(const void *data, uint32 size, void *priv);
    void OnAudioDecFrameReady(const void *data, uint32 size);
protected:
    CMediaPlayer();
    ~CMediaPlayer();
private:
    CMediaDecoder *m_lpMediaDecoder;
};

#define g_MediaPlayer    (*CMediaPlayer::instance())

#endif //__MEDIA_PLAYER_H__