#include "iAIUIImpl.h"

void AudioSDK_RegisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context)
{
    g_AIUIImpl.RegisterAudioMsgCallback(fun, context);
}

void AudioSDK_UnregisterAudioMsgCallback(AVDATA_CALLBACK_FUN fun, void * context)
{
    g_AIUIImpl.UnregisterAudioMsgCallback(fun, context);
}

void AudioSDK_RegisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context)
{
    g_AIUIImpl.RegisterStateChangeCallback(fun, context);
}

void AudioSDK_UnregisterStateChangeCallback(STATE_CHANGE_CALLBACK_FUN fun, void * context)
{
    g_AIUIImpl.UnregisterStateChangeCallback(fun, context);
}

