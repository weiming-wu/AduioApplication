/******************************************************************************

                  版权所有 (C), 2018-2099, 恒大集团

 ******************************************************************************
  文 件 名   : LuaOperate.cpp
  版 本 号   : 初稿
  作    者   : wuweiming
  生成日期   : 2019年9月30日
  最近修改   :
  功能描述   : c++条用lua相关操作

  修改历史   :
  1.日    期   : 2019年9月30日
    作    者   : wuweiming
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "LuaOperate.h"
#include "SysLog.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/


/*****************************************************************************
 函 数 名  : lua_op_init
 功能描述  : 初始化lua调用
 输入参数  : @param path: lua文件路径
 输出参数  : 无
 返 回 值  : @return: lua_State类型指针
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年9月30日
    作    者   : wuweiming
    修改内容   : 新生成函数

*****************************************************************************/
lua_State *lua_op_init(const char *path)
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, path);
    return L;
}

/*****************************************************************************
 函 数 名  : lua_op_cleanup
 功能描述  : 销毁lua调用
 输入参数  : @param L: lua_State类型指针
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年9月30日
    作    者   : wuweiming
    修改内容   : 新生成函数

*****************************************************************************/
void lua_op_cleanup(lua_State *L)
{
    lua_close(L);
}

/*****************************************************************************
 函 数 名  : call_lua_semantic_parse
 功能描述  : 调用lua进行语义解析
 输入参数  : @param L: lua_State类型指针
           @param str: 实际为需要转换的json格式字符串
           @param len: 输出buf的大小
 输出参数  : @param out_buf: 输出字符串缓冲buf
 返 回 值  : @return: 详见错误定义
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年9月30日
    作    者   : wuweiming
    修改内容   : 新生成函数

*****************************************************************************/
RETURN_TYPE_E call_lua_semantic_parse(lua_State *L, const char *str, char *out_buf, uint32 len)
{
    if ((NULL == L) || (NULL == str) || (NULL == out_buf) || (len <0))
    {
        LOG(ERROR, "Invalid param !\n");
        return RETURN_ERR_ARG;
    }
    lua_getglobal(L, "semantic_parse");

    lua_pushstring(L, str); //传入参数
    lua_pcall(L, 1, 1, 0);
    strncpy(out_buf, lua_tostring(L, -1), len);
    lua_pop(L, 1);
    LOG(INFO, "out_buf[%u]: %s\n", len, out_buf);

    //wwm + 2019-09-24 for test
    //remote_msgque_send(buf, strlen(buf));

    return RETURN_OK;
}

/*****************************************************************************
 函 数 名  : call_lua_to_evergrande
 功能描述  : 调用lua转换为恒大标准协议
 输入参数  : @param L: lua_State类型指针
           @param str: 实际为需要转换的json格式字符串
           @param method: 消息"method"字段内容
           @param req_id: 消息"req_id"字段内容
           @param user_id: 消息"user_id"字段内容
           @param dev_uuid: 消息"device_uuid"字段内容
           @param len: 输出消息buf大小

 输出参数  : @param out_buf: 输出字符串缓冲buf
 返 回 值  : @return: 详见错误定义
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2019年9月30日
    作    者   : wuweiming
    修改内容   : 新生成函数

*****************************************************************************/
RETURN_TYPE_E call_lua_to_evergrande(lua_State *L, const char *str, const char *method, int req_id,
    int user_id, const char *dev_uuid, char *out_buf, uint32 len)
{
    lua_getglobal(L, "airconditioner_semantic_to_evergrande");

    lua_pushstring(L, str); //传入参数
    lua_pushstring(L, method);
    lua_pushnumber(L, req_id);
    lua_pushnumber(L, user_id);
    lua_pushstring(L, dev_uuid);
    lua_pcall(L, 5, 1, 0);
    strncpy(out_buf, lua_tostring(L, -1), len);
    lua_pop(L, 1);
    LOG(INFO, "out_buf[%u]: %s\n", len, out_buf);

    //wwm + 2019-09-24 for test
    //remote_msgque_send(buf, strlen(buf));

    return RETURN_OK;
}

