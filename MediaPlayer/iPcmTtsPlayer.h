#ifndef __IPCM_TTS_PLAYER_H__
#define __IPCM_TTS_PLAYER_H__

#include "PcmTtsPlayerImpl.h"
#ifdef __cplusplus
extern "C"{
#endif

typedef enum
{
    PCM_PLAY_START = 0,
    PCM_PLAY_MID   = 1,
    PCM_PLAY_END   = 2,
}EN_PLAY_PROGRESS;

void DoPcmPlay(const char *data, uint32 len, int flag);


#ifdef __cplusplus
}
#endif

#endif