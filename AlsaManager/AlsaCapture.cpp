#include "AlsaCapture.h"
#include "SysLog.h"


PATTERN_SINGLETON_IMPLEMENT(CAlsaCapture);
CAlsaCapture::CAlsaCapture()
    : CAlsaInterface("6mic_loopback")
{
}

CAlsaCapture::~CAlsaCapture()
{
}

BOOL CAlsaCapture::init(uint32 rate, uint32 channels)
{
    if (!openDevice())
    {
        LOG(ERROR, "openDevice failed !\n");
        return FALSE;
    }
    return setParams(rate, channels);
}

BOOL CAlsaCapture::openDevice()
{
    int ret = snd_pcm_open(&m_pcmHandle, m_pcmDevice.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (ret < 0)
    {
        LOG(ERROR, "snd_pcm_open %s failed !\n", m_pcmDevice.c_str());
        return FALSE;
    }
    return TRUE;
}

BOOL CAlsaCapture::setParams(uint32 rate, uint32 channels)
{
    snd_pcm_hw_params_t *hw_params;
    int err;

    if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    if((err = snd_pcm_hw_params_any(m_pcmHandle, hw_params)) < 0)
    {
        fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    if ((err = snd_pcm_hw_params_set_access(m_pcmHandle, hw_params,
                                            SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        fprintf(stderr, "cannot set access type (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    if ((err = snd_pcm_hw_params_set_format(m_pcmHandle,
                                            hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        fprintf(stderr, "cannot set sample format (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    if ((err = snd_pcm_hw_params_set_channels(m_pcmHandle,
                                              hw_params, channels)) < 0)
    {
        fprintf(stderr, "cannot set channel count (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(m_pcmHandle,
                                               hw_params, &rate, 0)) < 0)
    {
        fprintf(stderr, "cannot set sample rate (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    snd_pcm_uframes_t period_frames = 512;
    if((err = snd_pcm_hw_params_set_period_size_near(m_pcmHandle,
              hw_params, &period_frames, 0)) < 0)
    {
        fprintf(stderr, "cannot set sample rate (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    if((err = snd_pcm_hw_params(m_pcmHandle, hw_params)) < 0)
    {
        fprintf(stderr, "cannot set parameters (%s)\n",
                snd_strerror(err));
        return FALSE;
    }

    snd_pcm_hw_params_free(hw_params);

    return TRUE;
}

int CAlsaCapture::readStream(void* buffer, int buff_size)
{
    return snd_pcm_readi(m_pcmHandle, buffer, buff_size);
}


