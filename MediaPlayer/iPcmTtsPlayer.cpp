#include <unistd.h>
#include "iPcmTtsPlayer.h"
#include "SysLog.h"

void DoPcmPlay(const char *data, uint32 len, int flag)
{
    if (PCM_PLAY_START == flag)
    {
        g_PcmTtsPlayerImpl.TtsPlay();
    }
    g_PcmTtsPlayerImpl.PushData(data, len);

    if (PCM_PLAY_END == flag)
    {
        //usleep(100000);
        g_PcmTtsPlayerImpl.TtsEnd();
        //g_PcmTtsPlayerImpl.TtsStop();
    }
}

