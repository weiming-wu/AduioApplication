/******************************************************************************

                  版权所有 (C), 2018-2099, 恒大集团

 ******************************************************************************
  文 件 名   : AudioMsgManager.cpp
  版 本 号   : 初稿
  作    者   : wuweiming
  生成日期   : 2019年10月3日
  最近修改   :
  功能描述   : 语音消息管理类

  修改历史   :
  1.日    期   : 2019年10月3日
    作    者   : wuweiming
    修改内容   : 创建文件

******************************************************************************/
#include <string>
#include <string.h>
#include "AudioMsgManager.h"
#include "SysLog.h"
#include "CJsonObject.hpp"
#include "iAIUIImpl.h"
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
static const char *lua_file_path[LUA_HND_IDX_MAX] =
{
    "Lua/conv.lua",
    "Lua/semantic_to_evergrande.lua"
};
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define AUDIO_MSG_READY    0x0101

PATTERN_SINGLETON_IMPLEMENT(CAudioMsgManager);
PATTERN_SINGLETON_IMPLEMENT(CAudioMsgSend);
CAudioMsgManager::CAudioMsgManager()
    : CThread(FALSE, 128, 0, "AudioMsgManager")
{
    for (uint32 i=0; i<ARRAY_SIZE(lua_file_path); i++)
    {
        m_luaHnd[i] = lua_op_init(lua_file_path[i]);
    }
}

CAudioMsgManager::~CAudioMsgManager()
{
    for (uint32 i=0; i<ARRAY_SIZE(lua_file_path); i++)
    {
        if (NULL != m_luaHnd[i])
        {
            lua_op_cleanup(m_luaHnd[i]);
        }
    }
}

BOOL CAudioMsgManager::Init()
{
    AudioSDK_RegisterAudioMsgCallback(&CAudioMsgManager::AudioMsgReady, this);

    return CreateThread();
}

BOOL CAudioMsgManager::CleanUp()
{
    AudioSDK_UnregisterAudioMsgCallback(&CAudioMsgManager::AudioMsgReady, this);
    return DestroyThread(TRUE);
}

int CAudioMsgManager::AudioMsgReady(const void * raw_data, void * priv)
{
    if(RETURN_OK == ((CAudioMsgManager *)priv)->OnAudioMsgReady(raw_data))
    {
        return 0;
    }
    return -1;
}

RETURN_TYPE_E CAudioMsgManager::OnAudioMsgReady(const void * raw_data)
{
    if (NULL == raw_data)
    {
        LOG(ERROR, "Invalid param !\n");
        return RETURN_ERR_ARG;
    }
    uint32 len = strlen((char *)raw_data);
    char *pmsg = (char *)malloc(len);
    if (NULL == pmsg)
    {
        LOG(ERROR, "Out of memory!\n");
        return RETURN_ERR_OOM;
    }
    memcpy(pmsg, (char *)raw_data, len);
    //LOG(INFO, "OnAudioMsgReady: %s\n", pmsg);
    if (FALSE == SendMsg(AUDIO_MSG_READY, len, (ulong)pmsg))
    {
        if (NULL != pmsg)
        {
            free(pmsg);
            pmsg = NULL;
        }
        return RETURN_ERR_MSG_SEND;
    }
    return RETURN_OK;
}

void CAudioMsgManager::ThreadProc()
{
    //语义转换等处理
    XMSG msg;

    while (m_bLoop)
    {
        if (RecvMsg(&msg, TRUE))
        {
            switch(msg.msg)
            {
                case AUDIO_MSG_READY:
                    {
                        //int len = msg.wpa;
                        //语音消息处理（转换），处理完了要手动释放内存free(msg.extra)
                        if (msg.lpa)
                        {
                            AudioMsgProcess((void *)msg.lpa);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            LOG(ERROR, "recv msg error\n");
            break;
        }
    }
}

RETURN_TYPE_E CAudioMsgManager::AudioMsgProcess(const void *msg, uint32 len/*=0*/)
{
    //这里先不区分音乐、天气和设备控制业务，只测试设备控制
    RETURN_TYPE_E ret = RETURN_ERR_UNKNOWN;

    cJSON *json = cJSON_Parse((char *)msg);
    if (NULL == json)
    {
        LOG(ERROR, "Parse json format error!\n");
        return RETURN_ERR_JSON_FORMAT;
    }
    free((char *)msg);
    msg = 0;
    cJSON *subJson = cJSON_GetObjectItem(json, "intent");
    if (NULL == subJson)
    {
        cJSON_Delete(json);
        LOG(ERROR, "Parse json format error!\n");
        return RETURN_ERR_JSON_FORMAT;
    }
    char *pstr = cJSON_Print(subJson); //cJSON_PrintUnformatted(json);
    LOG(INFO, "pstr: %s\n", pstr);
    if (pstr)
    {
        char tmp_buf[1024] = {0};
        memset(tmp_buf, 0, sizeof(tmp_buf));
        call_lua_semantic_parse(m_luaHnd[LUA_HND_IDX_STEP], pstr, tmp_buf, sizeof(tmp_buf)-1);
        free(pstr);
        pstr = NULL;
        {
            /* 最终转换为恒大标准协议 */
            char method[] = "dm_set";
            int req_id = 178237278;
            int user_id = 2003;
            char dev_uuid[] = "112233445566778810";
            char out_buf[1024] = {0};
            call_lua_to_evergrande(m_luaHnd[LUA_HND_IDX_FINAL], tmp_buf, method, req_id,
                user_id, dev_uuid, out_buf, sizeof(out_buf)-1);

            /* 转换完成后，发送给云端或者中控 */
        }
        ret = RETURN_OK;
    }

    cJSON_Delete(json);

    return ret;
}

#if 1
CAudioMsgSend::CAudioMsgSend()
    : CThread(FALSE, 128, 0, "AudioMsgSend")
{
}

CAudioMsgSend::~CAudioMsgSend()
{
}

BOOL CAudioMsgSend::Init()
{
    return CreateThread();
}

BOOL CAudioMsgSend::CleanUp()
{
    return DestroyThread(TRUE);
}

void CAudioMsgSend::ThreadProc()
{
    #if 0
    //int ret = -1;
    const char msg[]="{\
            \"category\": \"EVERGRANDE.airconditioner\",\
            \"intentType\": \"custom\",\
            \"rc\": 0,\
            \"semanticType\": 1,\
            \"service\": \"EVERGRANDE.airconditioner\",\
            \"uuid\": \"atn06a51f34@dx000610e38356a11101\",\
            \"semantic\": [\
                {\
                    \"entrypoint\": \"ent\",\
                    \"intent\": \"setOnOff\",\
                    \"score\": 0.8631743788719177,\
                    \"slots\": [\
                        {\
                            \"begin\": 0,\
                            \"end\": 2,\
                            \"name\": \"categoryName\",\
                            \"normValue\": \"空调\",\
                            \"value\": \"空调\"\
                        },\
                        {\
                            \"begin\": 2,\
                            \"end\": 3,\
                            \"name\": \"switch\",\
                            \"normValue\": \"打开\",\
                            \"value\": \"开\"\
                        },\
                        {\
                            \"begin\": 6,\
                            \"end\": 9,\
                            \"name\": \"room\",\
                            \"normValue\": \"客厅\",\
                            \"value\": \"客厅\"\
                        }\
                    ],\
                    \"template\": \"的{categoryName}{switch}\"\
                }\
            ]\
            }";
    //uint32 len = strlen(msg);
    while (m_bLoop)
    {
        m_lpAVDataNotifier.cb_fun(msg, m_lpAVDataNotifier.context);
        sleep(3);
    }
    #endif
}

#endif