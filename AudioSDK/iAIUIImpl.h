#ifndef __IAIUI_IMPL_H__
#define __IAIUI_IMPL_H__

#include "AIUIImpl.h"

#ifdef __cplusplus
extern "C"{
#endif

void AudioSDK_RegisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context);
void AudioSDK_UnregisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context);

void AudioSDK_RegisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context);
void AudioSDK_UnregisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context);
#ifdef __cplusplus
}
#endif

#endif