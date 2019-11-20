#ifndef __ALSA_PLAYBACK_H__
#define __ALSA_PLAYBACK_H__

#include "BaseFunction.h"
#include "AlsaInterface.h"

const int SND_PLAYBACK_SAMPLE_RATES = 44100;
const int SND_PLAYBACK_CHANNELS = 2;
const int PCM_NB_SAMPLES = 512;
const int PCM_SAMPLE_RATES = 16000;
const int PCM_FRAME_SIZE = 1024;
class CAlsaPlayback: public CAlsaInterface
{
    PATTERN_SINGLETON_DECLARE(CAlsaPlayback);
public:
    virtual BOOL init(uint32 rate, uint32 channels);
    BOOL writeStream(const void* buffer, ulong buff_size, uint32 channels=2);
protected:
    CAlsaPlayback();
    ~CAlsaPlayback();
protected:
    virtual BOOL openDevice();

    virtual BOOL setParams(uint32 rate, uint32 channels);

private:
    //snd_pcm_t* m_pcmHandle;

};
#define    g_AlsaPlayback    (*CAlsaPlayback::instance())
#endif