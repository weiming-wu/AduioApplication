#include "AlsaPlayback.h"
#include "SysLog.h"

#define ALSA_MAX_BUFFER_TIME 500000

PATTERN_SINGLETON_IMPLEMENT(CAlsaPlayback);
CAlsaPlayback::CAlsaPlayback()
    : CAlsaInterface("default")
{
}

CAlsaPlayback::~CAlsaPlayback()
{
}

BOOL CAlsaPlayback::init(uint32 rate, uint32 channels)
{
    if (!openDevice())
    {
        LOG(ERROR, "openDevice failed !\n");
        return FALSE;
    }
    return setParams(rate, channels);
}

BOOL CAlsaPlayback::openDevice()
{
    int ret = snd_pcm_open(&m_pcmHandle, m_pcmDevice.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0)
    {
        LOG(ERROR, "snd_pcm_open %s failed !\n", m_pcmDevice.c_str());
        return FALSE;
    }
    return TRUE;
}

BOOL CAlsaPlayback::setParams(uint32 rate, uint32 channels)
{
    snd_pcm_hw_params_t *params = nullptr;
    uint32_t buffer_time = 0;
    uint32_t period_time = 0;
    int ret = 0;
    snd_pcm_hw_params_alloca(&params);

    ret = snd_pcm_hw_params_any(m_pcmHandle, params);
    if (ret < 0)
    {
        return FALSE;
    }
    ret = snd_pcm_hw_params_set_access(m_pcmHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0)
    {
        return FALSE;
    }
    ret = snd_pcm_hw_params_set_format(m_pcmHandle, params, SND_PCM_FORMAT_S16_LE);
    if (ret < 0)
    {
        return FALSE;
    }
    ret = snd_pcm_hw_params_set_channels(m_pcmHandle, params, channels);
    if (ret < 0)
    {
        return FALSE;
    }
    ret = snd_pcm_hw_params_set_rate_near(m_pcmHandle, params, &rate, nullptr);
    if (ret < 0)
    {
        return FALSE;
    }
    ret = snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, nullptr);
    if (ret < 0)
    {
        return FALSE;
    }
    buffer_time = buffer_time > ALSA_MAX_BUFFER_TIME ? ALSA_MAX_BUFFER_TIME : buffer_time;
    period_time = buffer_time / 4;
    ret = snd_pcm_hw_params_set_buffer_time_near(m_pcmHandle, params, &buffer_time, nullptr);
    if (ret < 0)
    {
        return FALSE;
    }
    ret = snd_pcm_hw_params_set_period_time_near(m_pcmHandle, params, &period_time, nullptr);
    if (ret < 0) {
        return FALSE;
    }

    ret = snd_pcm_hw_params(m_pcmHandle, params);
    if (ret < 0)
    {
        return FALSE;
    }
    m_alsaCanPause = snd_pcm_hw_params_can_pause(params);

    snd_pcm_uframes_t chunk_size = 0;
    snd_pcm_uframes_t buffer_size = 0;
    ret = snd_pcm_hw_params_get_period_size(params, &chunk_size, nullptr);
    if (ret < 0)
    {
        return FALSE;
    }
    ret = snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
    if (ret < 0)
    {
        return FALSE;
    }
    m_chunkBytes = chunk_size * snd_pcm_format_physical_width(SND_PCM_FORMAT_S16_LE) / 8;

    return TRUE;
}

BOOL CAlsaPlayback::writeStream(const void* buffer, ulong buff_size, uint32 channels)
{
    CGuard g(m_Mutex);
    if (m_abortFlag || m_pcmHandle == NULL)
    {
        LOG(WARNING, "skip !\n");
        return FALSE;
    }

    int ret = 0;
    //LOG(INFO, "writeStream buff_size: %lu\n", buff_size);
    while ((ret = snd_pcm_writei(m_pcmHandle, buffer, buff_size / channels / 2)) < 0)
    {
        if (m_abortFlag)
        {
            break;
        }
        else
        {
            snd_pcm_prepare(m_pcmHandle);
            //snd_pcm_recover(m_pcmHandle, ret, 0); //ALSA lib pcm.c:7843:(snd_pcm_recover) underrun occurred
        }
    }

    return TRUE;
}