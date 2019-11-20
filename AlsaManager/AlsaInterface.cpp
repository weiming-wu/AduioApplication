#include "AlsaInterface.h"
#include "SysLog.h"

CAlsaInterface::CAlsaInterface(const std::string& audio_dev)
    : m_pcmHandle(NULL)
{
    m_pcmDevice = audio_dev;
}

BOOL CAlsaInterface::isAccessable()
{
    CGuard g(m_Mutex);
    if (NULL == m_pcmHandle)
    {
        LOG(INFO, "m_pcmHandle is null !\n");
        return FALSE;
    }
    return TRUE;
}

BOOL CAlsaInterface::alsaPrepare()
{
    CGuard g(m_Mutex);
    if (NULL == m_pcmHandle)
    {
        LOG(INFO, "m_pcmHandle is null !\n");
        return FALSE;
    }

    snd_pcm_prepare(m_pcmHandle);
    //m_abortFlag = false;
    return TRUE;
}

BOOL CAlsaInterface::alsaPause()
{
    CGuard g(m_Mutex);
    int err = 0;
    if (m_alsaCanPause)
    {
        if ((err = snd_pcm_pause(m_pcmHandle, 1)) < 0)
        {
            return TRUE;
        }
    }
    else
    {
        if ((err = snd_pcm_drop(m_pcmHandle)) < 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CAlsaInterface::alsaResume()
{
    CGuard g(m_Mutex);
    int err = 0;
    if (snd_pcm_state(m_pcmHandle) == SND_PCM_STATE_SUSPENDED)
    {
        while ((err = snd_pcm_resume(m_pcmHandle)) == -EAGAIN)
        {
            sleep(1);
        }
    }

    if (m_alsaCanPause)
    {
        if ((err = snd_pcm_pause(m_pcmHandle, 0)) < 0)
        {
            return TRUE;
        }
    }
    else
    {
        if ((err = snd_pcm_prepare(m_pcmHandle)) < 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CAlsaInterface::aslaAbort()
{
    CGuard g(m_Mutex);
    if (m_pcmHandle != nullptr)
    {
        m_abortFlag = TRUE;
        snd_pcm_abort(m_pcmHandle);
        return TRUE;
    }
    return FALSE;
}

BOOL CAlsaInterface::alsaClear()
{
    CGuard g(m_Mutex);
    if (nullptr != m_pcmHandle)
    {
        snd_pcm_drop(m_pcmHandle);
        return TRUE;
    }
    return FALSE;
}

BOOL CAlsaInterface::alsaClose()
{
    CGuard g(m_Mutex);
    if (nullptr == m_pcmHandle)
    {
        return FALSE;
    }
    snd_pcm_drop(m_pcmHandle);
    snd_pcm_close(m_pcmHandle);
    m_pcmHandle = nullptr;
    return TRUE;
}

