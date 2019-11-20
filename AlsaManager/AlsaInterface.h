#ifndef __ALSA_INTERFACE_H__
#define __ALSA_INTERFACE_H__

#include <string>
#include <alsa/asoundlib.h>
#include "Guard.h"
#include "Mutex.h"

class CAlsaInterface
{
public:
    CAlsaInterface(const std::string& audio_dev);

    virtual ~CAlsaInterface(){};

    virtual BOOL init(uint32 rate, uint32 channels) = 0;

    BOOL isAccessable();

    BOOL alsaPrepare();

    BOOL alsaPause();

    BOOL alsaResume();

    BOOL aslaAbort();

    BOOL alsaClear();

    BOOL alsaClose();

protected:
    virtual BOOL openDevice() = 0;

    virtual BOOL setParams(uint32 rate, uint32 channels) = 0;

protected:
    std::string m_pcmDevice;
    snd_pcm_t* m_pcmHandle;
    CMutex    m_Mutex;
    int m_alsaCanPause;
    BOOL m_abortFlag;
    size_t m_chunkBytes;
};
#endif
