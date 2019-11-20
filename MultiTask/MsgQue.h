#ifndef __MSG_QUE_H__
#define __MSG_QUE_H__

#include <list>
#include "Mutex.h"
#include "Semaphore.h"
#include "Type.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum xm_msg_t
{
    XM_SYSTEM = 0,
    XM_QUIT,
    XM_TIMER,       //定时器
    XM_PLAYBACK_PLAY, //wpa:session
    XM_PLAYBACK_STOP, //wpa:session
    XM_PLAYBACK_PAUSE, //wpa:session
    XM_PLAYBACK_RESUME, //wpa:session
    XM_PLAYBACK_SEEK_TO, //wpa:session,lpa:videoFrameNo
    XM_PLAYBACK_RESEND_AUDIO_FRAME, //wpa:session,lpa:audioFrameNo
    XM_PLAYBACK_RESEND_VIDEO_FRAME, //wpa:session,lpa:videoFrameNo
    XM_GPS_MODULE_ACTIVE,   //wpa:gps module active, lpa:unused
    XM_DISPLAY_GPS_SPEED_OSD,   //wpa:是否显示　lpa:unused
    XM_GPS_SPEED,   //wpa:unused lpa:GPS速度(km/s)
    XM_UPDATE_TIMESTAMP,    //wpa:unused lpa:unused
    XM_UPDATE_TIMESTAMP2,   //wpa:unused lpa:当前时刻(s)
    XM_UPDATE_ISP_DEBUG_INFO_OSD,   //wpa,lpa按实际需求定义和解析
    XM_CLOUD_NOTIFY_RECORD_FILE, //云端通知录像文件 wpa: unused lpa:CNotifyFileNode*
    XM_P2P_RELEASE_CLIENT, //wpa:unused, lpa:CP2PC*
} emXMsg;

typedef enum
{
    MSG_PRIORITY_HIGHTEST = 0,
    MSG_PRIORITY_NORMAL,
    //MSG_PRIORITY_LOWEST,
    MSG_PRIORITY_ILL
} emMsgPriority;

typedef struct tagXMSG
{
    uint32        msg;
    uint32        wpa;
    uint64        lpa;
    uint32        time;
} XMSG;

typedef std::list<XMSG/*, pool_allocator<XMSG> */> MSGQUEUE;

class CMsgQue
{
public:
    CMsgQue(uint32 size = 1024);
    virtual ~CMsgQue();

    BOOL SendMsg(uint32 msg, uint32 wpa = 0, ulong lpa = 0, uint32 priority = MSG_PRIORITY_NORMAL);
    BOOL RecvMsg(XMSG *pMsg, BOOL wait = TRUE);
    void QuitMsg();
    void ClearMsg();
    uint32 GetMsgCount();
    uint32 GetMsgSize();
    void SetMsgSize(uint32 size);

protected:
private:
    MSGQUEUE m_Queue;
    BOOL m_bMsgFlg;
    CMutex m_Mutex;
    CSemaphore m_Semaphore;
    uint32 m_nMsg;
    uint32 m_nMaxMsg;
};

#endif// __MSG_QUE_H__

