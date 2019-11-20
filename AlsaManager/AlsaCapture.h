#ifndef __ALSA_CAPTURE_H__
#define __ALSA_CAPTURE_H__

#include "BaseFunction.h"
#include "AlsaInterface.h"

class CAlsaCapture: public CAlsaInterface
{
    PATTERN_SINGLETON_DECLARE(CAlsaCapture);
public:
    virtual BOOL init(uint32 rate, uint32 channels);
    int readStream(void* buffer, int buff_size);
protected:
    CAlsaCapture();
    ~CAlsaCapture();
protected:
    virtual BOOL openDevice();

    virtual BOOL setParams(uint32 rate, uint32 channels);
};

#define g_AlsaCapture    (*CAlsaCapture::instance())
#endif