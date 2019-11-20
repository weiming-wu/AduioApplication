#ifndef _IOT_MSG_UTIL_H_
#define _IOT_MSG_UTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/* 消息队列编号 */
typedef enum
{
  /*  IOT_MSG_SQUEUE_EVENT_LOOP, */   /* 消息队列整改后，即删除 */
    IOT_MSG_SQUEUE_LOCAL,         /* evenloop与近程协议模块之间的消息队列，
                                     包括tcp_server、http */
    IOT_MSG_SQUEUE_REMOTE,        /* evenloop与远程协议模块之间的消息队列，
                                     包括tcp_client、telnet */
    IOT_MSG_SQUEUE_WIFI,          /* evenloop与wifi模块之间的消息队列 */
    IOT_MSG_SQUEUE_ZIGBEE,        /* evenloop与zigbee模块之间的消息队列 */
    IOT_MSG_SQUEUE_ROUTER,        /* evenloop与一般业务模块之间的消息队列，
                                     包括router、media、ota、device、storage */
    IOT_MSG_SQUEUE_DATABASE,      /* evenloop与db模块之间的消息队列 */
    IOT_MSG_SQUEUE_DEBUG,         /* log模块与其他模块之间的消息队列 */
    IOT_MSG_SQUEUE_LOG_DEBUG,         /* msg queue of operation record*/
    IOT_MSG_SQUEUE_MEDIA_WIFI,
    IOT_MSG_SQUEUE_CGI,
    /* wwm + 2019-09-24 */
    IOT_MSG_SQUEUE_AUDIO_PLATFORM,
}IOT_MSG_SQUEUE_NO;

/* 模块ID，有增删时，需同步更新 IOT_MODULE_ID_NUM 值 */
typedef enum
{
    IOT_MODULE_NONE         = 0x00,
    IOT_MODULE_EVENT_LOOP   = 0x01,
    IOT_MODULE_OTA_MGMT     = 0x02,
    IOT_MODULE_TELNET_DEBUG = 0x04,
    IOT_MODULE_WIFI         = 0x08,
    IOT_MODULE_ZIGBEE       = 0x10,
    IOT_MODULE_DEBUG        = 0x20,
    IOT_MODULE_HTTP         = 0x40,
    IOT_MODULE_ROUTER_MGMT  = 0x80,
    IOT_MODULE_STORAGE_MGMT = 0x100,
    IOT_MODULE_DEVICE_MGMT  = 0x200,
    IOT_MODULE_SEMANTIC_TRANSLATION   = 0x400,
    IOT_MODULE_TCP_SERVER   = 0x800,
    IOT_MODULE_TCP_CLIENT   = 0x1000,
    IOT_MODULE_TCP_DATABASE = 0x2000,
    IOT_MODULE_BT_MGMT      = 0x4000,
    IOT_MODULE_LED_SOUND    = 0x8000, // AI router, main board LED&SOUND module.
    IOT_MODULE_SEMANTIC_PARSER    = 0x10000,
    IOT_MODULE_TUTK_MGR     = 0x20000,
    IOT_MODULE_MEDIA_VI     = 0x40000, // video intercom
    IOT_MODULE_HTTP_CGI     = 0x80000, // cgi transfer module no need send module message
    IOT_MODULE_MEDIA_DB     = 0x100000, // doorbell
}IOT_MODULE_ID;

/* 模块ID个数，需与 IOT_MODULE_ID 同步 */
#define IOT_MODULE_ID_NUM 19

typedef enum
{
    IOT_HANDLE_NONE                   = 0x00000000,
    IOT_HANDLE_CLIENT_UNICAST_MODE    = 0x01000000,
    IOT_HANDLE_CLIENT_BROADCAST_MODE  = 0x02000000,
    IOT_HANDLE_SERVER_UNICAST_MODE    = 0x00000100,
    IOT_HANDLE_SERVER_BROADCAST_MODE  = 0x00000200,
    IOT_HANDLE_SERVER_MULTICAST_MODE  = 0x00000400,
}IOT_HANDLE_MODE_ID;


#define IOT_MSG_BODY_LEN_MAX  8000

typedef struct
{
	long dst_mod;
    long src_mod;
    long handler_mod;
    unsigned int body_len;
    unsigned char body[IOT_MSG_BODY_LEN_MAX];
}IOT_MSG;

enum UPGRADE_PKG_TYPE
{
    RT_ANDROID_PKG = 1,
    RT_ROUTER_PKG,
    RT_DEVICE_PKG,
    RT_IOS_PKG,
    RT_H5_PKG
};


/*
  get or create a msg queue

  [in]queue_no: queue number to get
  [out]return:     failed == -1; success != -1
*/
int iot_msg_queue_get(IOT_MSG_SQUEUE_NO queue_no);

/*
  put msg to a msg queue

  [in]queue_id:         iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:     the module id to send
  [in]msg->src_mod:     msg creator's module id
  [in]msg->handler_mod: the module need to deal the msg(can set to none)
  [in]msg->body_len:the msg body's length
  [in]msg->body:    the msg body's context
  [out]return:          failed == -1; success == 0
*/
int iot_msg_send(int queue_id, IOT_MSG * msg);
int iot_msg_send_nowait(int queue_id, IOT_MSG * msg);

/*
  get msg from a msg squeue, if none, waiting

  [in]queue_id:          iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:      the module id to send.(aller's module id)
  [out]msg->src_mod:     msg creator's module id
  [out]msg->handler_mod: the module need to deal the msg(can set to none)
  [out]msg->body_len:    the msg body's length
  [out]msg->body:        the msg body's context
  [out]return:           failed < 0; success == msg len
*/
int iot_msg_recv_wait(int queue_id, IOT_MSG * msg_buf);

/*
  get msg from a msg squeue, if none, return

  [in]queue_id:          iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:      the module id to send.(aller's module id)
  [out]msg->src_mod:     msg sender's module id
  [out]msg->handler_mod: the module need to deal the msg(can set to none)
  [out]msg->body_len:    the msg body's length
  [out]msg->body:        the msg body's context
  [out]return:           failed < 0; success == msg len or 0
*/
int iot_msg_recv_nowait(int queue_id, IOT_MSG * msg_buf);

/*
  get msg from a msg squeue, if too long will be cut off

  [in]queue_id:          iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:      the module id to send.(aller's module id)
  [out]msg->src_mod:     msg sender's module id
  [out]msg->handler_mod: the module need to deal the msg(can set to none)
  [out]msg->body_len:    the msg body's length
  [out]msg->body:        the msg body's context
  [out]return:           failed < 0; success == msg len or 0
*/
int iot_msg_recv_log_wait(int queue_id, IOT_MSG *msg_buf);

#endif
