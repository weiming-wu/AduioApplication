#include "iot_msg_util.h"

#define IOT_MSG_REAL_LEN(msg) ((size_t)((((IOT_MSG*)0)->body)+(msg)->body_len-sizeof(long)))
#define IOT_MSG_MAX_LEN (sizeof(IOT_MSG)-sizeof(long))


int iot_msg_queue_get(IOT_MSG_SQUEUE_NO queue_no)
{
    //key_t key = ftok("/usr/lib/libiot_util.so", queue_no);
    key_t key = ftok("./../tmp", queue_no);
    if(key == -1)
    {
            printf("libiot_util: ftok get msg key failed, err=%s\n", strerror(errno));
            return -1;
    }

    int qid = msgget(key, 0);
    if(qid == -1)
    {
           qid = msgget(key, IPC_CREAT|0666);
    }

    return qid;
}

int iot_msg_send(int queue_id, IOT_MSG * msg)
{
    int len = IOT_MSG_REAL_LEN(msg);

    if(len > IOT_MSG_MAX_LEN)
    {
        len = IOT_MSG_MAX_LEN;
    }
    int ret = msgsnd(queue_id, msg, len, 0);
    if(ret == -1){
          printf("libiot_util: iot msg send failed, err=%s\n", strerror(errno));
    }
    return ret;
}

int iot_msg_send_nowait(int queue_id, IOT_MSG * msg)
{
    int len = IOT_MSG_REAL_LEN(msg);

    if(len > IOT_MSG_MAX_LEN)
    {
        len = IOT_MSG_MAX_LEN;
    }

    int ret = msgsnd(queue_id, msg, len, IPC_NOWAIT);
    if(ret == -1){
         printf("libiot_util: iot msg send failed queue_id=%d, src_mod=0x%lx, dst_mod=0x%lx,err=%s\n", queue_id, msg->src_mod, msg->dst_mod,strerror(errno));
    }
    return ret;
}

int iot_msg_recv_wait(int queue_id, IOT_MSG * msg)
{
    int ret = msgrcv(queue_id, msg, IOT_MSG_MAX_LEN, msg->dst_mod, 0);
    if(ret < 0){
        printf("libiot_util: iot msg recv (wait) failed, err=%s\n", strerror(errno));
    }
    return ret;
}

int iot_msg_recv_nowait(int queue_id, IOT_MSG * msg)
{
    int ret = msgrcv(queue_id, msg, IOT_MSG_MAX_LEN, msg->dst_mod,IPC_NOWAIT|MSG_NOERROR);
    if(ret<0 && errno!=ENOMSG){
        printf("libiot_util: iot msg recv (nowait) failed, err=%s\n", strerror(errno));
    }
    return ret;
}

int iot_msg_recv_log_wait(int queue_id, IOT_MSG *msg)
{
    int ret = msgrcv(queue_id, msg, IOT_MSG_MAX_LEN, msg->dst_mod, MSG_NOERROR);
    if(ret<0 && errno!=ENOMSG){
        printf("libiot_util: iot msg recv (nowait) failed, err=%s\n", strerror(errno));
    }
    return ret;
}