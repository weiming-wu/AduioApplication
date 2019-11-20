#include <stdio.h>
#include <stdlib.h>
#include "AudioMsgManager.h"
#include "SysLog.h"
#include "AIUIImpl.h"
#include "cae.h"
#include "AlsaCapture.h"
#include "AlsaPlayback.h"
#include "PcmTtsPlayerImpl.h"
//#include "iPcmTtsPlayer.h"
#include "PcmFilePlayerImpl.h"
#include "Mp3Decoder.h"
#include "MediaDecoder.h"
#include "DlnaDmrSdk.h"
using namespace std;
using namespace aiui;

BOOL g_bSystemQuit = FALSE;

void playPcmStream(const void *data, uint32 size, void *priv)
{
    g_AlsaPlayback.writeStream(data, size);
}

#if 0
int main(int argc, char *argv[])
{
    LOG(INFO, "%s build date: " __DATE__ " " __TIME__ "\n", argv[0]);
    do
    {
        //InitSignals();

        /* init audio capture */
        if (!g_AlsaCapture.init(16000, 8))
        {
            LOG(ERROR, "alsa capture init failed !\n");
            break;
        }

        /* init audio playback */
        //if (!g_AlsaPlayback.init(16000, 1))
        if (!g_AlsaPlayback.init(SND_PLAYBACK_SAMPLE_RATES, SND_PLAYBACK_CHANNELS))
        {
            LOG(ERROR, "alsa playback init failed!\n");
            break;
        }

        if (!g_AIUIImpl.Init())
        {
            LOG(ERROR, "aiui init failed !\n");
            break;
        }

        if (!g_PcmTtsPlayerImpl.Init())
        {
            LOG(ERROR, "pcm tts player init failed !\n");
            break;
        }

        if (!g_CAE.Init())
        {
            LOG(ERROR, "cae init failed !\n");
            break;
        }

        if (!g_AudioMsgManager.Init())
        {
            LOG(ERROR, "audio msg manager init failed !\n");
            break;
        }

        #if defined _DEBUG_DLNA
            duerOSDcsApp::dueros_dlna::DlnaDmrSdk dlnaDmrSdk;
            printf("##### wwm debug dlna drm sdk start...\n");
            dlnaDmrSdk.start();
        #endif
#if 0
        //Mp3Decoder decoder("http://static.soundai.cn:23080/kuwo/%E6%9C%A8%E9%B1%BC%E5%92%8C%E9%87%91%E9%B1%BC.mp3");
        Mp3Decoder decoder("connect.mp3");
        decoder.setPutBufferImpl(playPcmStream, NULL);
        decoder.prepare();
        decoder.start();
#else
/*
        CMediaDecoder dec("http://static.soundai.cn:23080/kuwo/%E6%9C%A8%E9%B1%BC%E5%92%8C%E9%87%91%E9%B1%BC.mp3");
        dec.RegisterDecCallback(playPcmStream, &dec);
        if(dec.Init())
        {
            sleep(1);
            dec.Start();
        }
*/
#endif
#if defined (_DEBUG_PLAYBACK)
        CPcmFilePlayerImpl filePlay("tts.pcm");
        if (filePlay.Init())
        {
            LOG(INFO, "pcm file player init ok.\n");
        }
        else
        {
            LOG(ERROR, "pcm file player init failed !\n");
        }
        sleep(2);
        filePlay.StartPlay();
#endif
#if defined(_DEBUG)
        g_AIUIImpl.test();
#endif

        //g_AudioMsgManager.Init();
        //g_AudioMsgSend.Init();
        //lua_op_init("lua/conv.lua");
        //g_CAE.Init();

        //EN_DECODE_STATE state;
        while (!g_bSystemQuit)
        {
            sleep(5);
            #if 0
            state = dec.GetDecodeState();
            if ((START_DECODE == state) ||
                (RESUME_DECODE == state))
            {
                LOG(INFO, "SET PAUSE\n");
                dec.Pause();
            }
            else if(PAUSE_DECODE == state)
            {
                LOG(INFO, "SET RESUME\n");
                dec.Resume();
            }
            #endif
        }
        #if defined (_DEBUG_PLAYBACK)
        filePlay.CleanUp();
        #endif
    }while(0);

    g_AudioMsgManager.CleanUp();
    g_PcmTtsPlayerImpl.CleanUp();
    g_CAE.CleanUp();
    g_AIUIImpl.CleanUp();
    //g_AudioMsgSend.CleanUp();

    return 0;
}
#else
int main(int argc, char *argv[])
{
    LOG(INFO, "%s build date: " __DATE__ " " __TIME__ "\n", argv[0]);
    #if defined _DEBUG_DLNA
        duerOSDcsApp::dueros_dlna::DlnaDmrSdk dlnaDmrSdk;
        printf("##### wwm debug dlna drm sdk start...\n");
        dlnaDmrSdk.start();
    #endif
    while(1)
    {
        sleep(1);
    }
}
#endif
