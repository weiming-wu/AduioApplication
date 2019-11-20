#include <unistd.h>
#include "MediaPlayer.h"
#include "AlsaPlayback.h"
#include "SysLog.h"

PATTERN_SINGLETON_IMPLEMENT(CMediaPlayer);

CMediaPlayer::CMediaPlayer()
        : m_lpMediaDecoder(NULL)
{
}

CMediaPlayer::~CMediaPlayer()
{
}

void CMediaPlayer::AudioDecFrameReady(const void *data, uint32 size, void *priv)
{
    ((CMediaPlayer *)priv)->OnAudioDecFrameReady(data, size);
}

void CMediaPlayer::OnAudioDecFrameReady(const void *data, uint32 size)
{
    g_AlsaPlayback.writeStream(data, size);
}

BOOL CMediaPlayer::PlayAudioStream(const char *url)
{
    if(NULL != m_lpMediaDecoder)
    {
        m_lpMediaDecoder->CleanUp();
        delete m_lpMediaDecoder;
        m_lpMediaDecoder = NULL;
    }
    m_lpMediaDecoder = new CMediaDecoder(url);
    if (NULL == m_lpMediaDecoder)
    {
        LOG(ERROR, "m_lpMediaDecoder instance failed !\n");
        return FALSE;
    }

    if (FALSE == m_lpMediaDecoder->Init())
    {
        LOG(ERROR, "Media decoder init failed !\n");
        return FALSE;
    }
    m_lpMediaDecoder->RegisterDecCallback(AudioDecFrameReady, this);
    //usleep(100000);//延时100ms避免media decoder线程还未运行就开始播放
    sleep(1);
    m_lpMediaDecoder->Start();

    return TRUE;
}

BOOL CMediaPlayer::PauseAudioStream()
{
    if(NULL == m_lpMediaDecoder)
    {
        LOG(ERROR, "m_lpMediaDecoder is null !\n");
        return FALSE;
    }
    m_lpMediaDecoder->Pause();

    return TRUE;
}

BOOL CMediaPlayer::ResumeAudioStream()
{
    if(NULL == m_lpMediaDecoder)
    {
        LOG(ERROR, "m_lpMediaDecoder is null !\n");
        return FALSE;
    }
    m_lpMediaDecoder->Resume();

    return TRUE;
}

BOOL CMediaPlayer::StopAudioStream()
{
    if(NULL == m_lpMediaDecoder)
    {
        LOG(ERROR, "m_lpMediaDecoder is null !\n");
        return FALSE;
    }
    m_lpMediaDecoder->Stop();
    usleep(100000);//延时100ms避免media decoder线程还未退出
    m_lpMediaDecoder->CleanUp();
    delete m_lpMediaDecoder;
    m_lpMediaDecoder = NULL;
    return TRUE;
}

