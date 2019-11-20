#ifndef __TASK_QUE_H__
#define __TASK_QUE_H__
#include <stdio.h>

#include "Mutex.h"
#include "Guard.h"
#include "Semaphore.h"


typedef enum
{
    TASKTYPE_SYSTEM = 0,
    TASKTYPE_QUIT,              /*退出线程*/
    TASKTYPE_TIMER,             /*定时器*/
    TASKTYPE_HEART_BEAT,        /*心跳事件*/
    TASKTYPE_ALARM,             /*报警事件*/
    TASKTYPE_SEND_FILE,         /*文件发送*/
    TASKTYPE_PLAY_AAC,          /*播放AAC音频*/
    TASKTYPE_ILL
} emTASKTYPE;

enum
{
    TASK_PRIORITY_HIGHTEST = 0,
    TASK_PRIORITY_NORMAL
};

typedef struct tagTimerContext
{
    uint    wpa;
    ulong   lpa;
} TimerContext;

typedef struct tagHeartBeatContext
{
    uint    wpa;
    ulong   lpa;
} HeartBeatContext;

typedef struct tagAlarmContext
{
    uint    type;   //报警类型
    uint    time;   //报警发生时间
    uint    wpa;    //附加信息1
    ulong   lpa;    //附加信息2
} AlarmContext;

typedef struct tagVoiceTrainContext
{
    int     index;//0~4
    char    name[64];
    uint    linkage;
    uint    wpa; //附加信息1,0x1:第一次开启训练,0x2:继续训练,0x4:取消训练
    ulong   lpa; //附加信息2,取消训练时是否切换到识别模式(如果已经有训练好的语音则切换,否则不切换)
} VoiceTrainContext;

typedef struct tagVoiceTrainDoneContext
{
    uint    wpa;    //附加信息1(是否继续训练【否则切换到识别模式】)
    ulong   lpa;    //附加信息2
} VoiceTrainDoneContext;

typedef struct tagSendFileContext
{
    char    filename[64];
    char    name[64];
    int     offset; //文件偏移量
    int     size;  //要发送的大小,如果为0 则发送全部
    BOOL    encrypt;
} SendFileContext;

typedef struct tagPlayAACData
{
    void   *data;
    int     size;
    BOOL    mix;
    int     mixvol;
    uint    duration;
} PlayAACDataContext; //extra为上下文信息

typedef struct tagTASK
{
    int                 type;       // emTASKTYPE
    int                 priority;   // 任务优先级
    uint                priv;       // 私有信息(区分/查找任务)
    union
    {
        TimerContext        timer;              // extra为NULL
        HeartBeatContext    heartbeat;          // extra为NULL
        AlarmContext        alarm;              // extra为NULL
        VoiceTrainContext   voicetrain;         // extra为NULL
        VoiceTrainDoneContext   voicetraindone; // extra为NULL
        SendFileContext     sendfile;
        PlayAACDataContext  playaac;
    } context;
    void               *extra;      // 其他上下文信息
} TASK;


typedef void *TaskToken_t;
#define INVALID_TASKTOKEN ((TaskToken_t)0)

typedef int (* SPECTASKPROC)(TASK *task);
class CTaskNodeManager;
class CTaskQue
{
public:
    CTaskQue(uint maxTasks);
    ~CTaskQue(void);
public:
    TaskToken_t             PutTask(TASK *task, int priority = TASK_PRIORITY_NORMAL);
    BOOL                    GetTask(TASK *task, BOOL bWait = TRUE);
    BOOL                    RemoveTaskByPrivData(uint priv, SPECTASKPROC proc);
private:
    uint                    m_nMaxTasks;
    CTaskNodeManager       *m_lpTaskNodeManager;
};

#endif
