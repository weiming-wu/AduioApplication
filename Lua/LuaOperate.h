#ifndef __LUA_OPERATE_H__
#define __LUA_OPERATE_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "Type.h"

lua_State *lua_op_init(const char *path);

void lua_op_cleanup(lua_State *L);

RETURN_TYPE_E call_lua_semantic_parse(lua_State *L, const char *str, char *out_buf, uint32 len);

RETURN_TYPE_E call_lua_to_evergrande(lua_State *L, const char *str, const char *method, int req_id,
    int user_id, const char *dev_uuid, char *out_buf, uint32 len);
#ifdef __cplusplus
}
#endif
#endif