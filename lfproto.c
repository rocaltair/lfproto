/*
 * lfproto, infos of a lua function
 *
 * Copyright (C) 2016, zhupeng<rocaltair@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <lua.h>
#include <lauxlib.h>
#include "lstate.h"
#include "lobject.h"
#include "lfunc.h"

#if LUA_VERSION_NUM < 502 && (!defined(luaL_newlib))
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#endif

#define LFUNC_PROTO "lfunc.proto"

#define getlproto(L, n) (clvalue((L)->base + n - 1)->l.p)
#define islfunc(L, n) (lua_isfunction(L, n) && !lua_iscfunction(L, n))
#define lfCheckProto(L, n) ((islfunc(L, n) || luaL_error(L, "lua function required")), getlproto(L, n))

#define checkproto(L, idx) (*(struct Proto **) luaL_checkudata(L, idx, LFUNC_PROTO))


static int lua__proto(lua_State *L)
{
	struct Proto *ptr = lfCheckProto(L, 1);
	struct Proto **p = (struct Proto **)lua_newuserdata(L, sizeof(*p));
	*p = ptr;
	luaL_getmetatable(L, LFUNC_PROTO);
	lua_setmetatable(L, -2);
	return 1;
}

static int lua__line(lua_State *L)
{
	struct Proto *p = checkproto(L, 1);
	lua_pushnumber(L, p->linedefined);
	lua_pushnumber(L, p->lastlinedefined);
	return 2;
}

static int lua__locals(lua_State *L)
{
	int i, n;
	struct Proto *p = checkproto(L, 1);
	lua_newtable(L);
	for (n = 0, i = p->numparams; i < p->sizelocvars; i++) {
  		const char * name = getstr(p->locvars[i].varname);
		int startpc = p->locvars[i].startpc;
		int endpc = p->locvars[i].endpc;
		int sline = p->lineinfo[startpc];
		int eline = p->lineinfo[endpc];

		lua_newtable(L);
		lua_pushstring(L, name);
		lua_setfield(L, -2, "name");
		lua_pushnumber(L, sline);
		lua_setfield(L, -2, "first");
		lua_pushnumber(L, eline);
		lua_setfield(L, -2, "last");

		lua_rawseti(L, -2, ++n);
	}
	return 1;
}

static int lua__upvalues(lua_State *L)
{
	int i;
	struct Proto *p = checkproto(L, 1);
	lua_newtable(L);
	for (i = 0; i < p->nups; i++) {
  		const char * name = getstr(p->upvalues[i]);
		lua_pushstring(L, name);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int lua__source(lua_State *L)
{
	struct Proto *p = checkproto(L, 1);
	lua_pushstring(L, getstr(p->source));
	return 1;
}

static int lua__nparams(lua_State *L)
{
	struct Proto *p = checkproto(L, 1);
	lua_pushnumber(L, p->numparams);
	return 1;
}

static int lua__is_vararg(lua_State *L)
{
  	struct Proto *p = checkproto(L, 1);
	lua_pushboolean(L, p->is_vararg);
	return 1;
}


static int lua__paralist(lua_State *L)
{
	int i;
	struct Proto *p = checkproto(L, 1);
	lua_newtable(L);
	for (i = 0; i < p->numparams; i++) {
  		const char * name = getstr(p->locvars[i].varname);
		lua_pushstring(L, name);
		lua_rawseti(L, -2, i + 1);
	}
	lua_pushboolean(L, p->is_vararg);
        return 2;
}

static int lua__protos(lua_State *L)
{
	int i;
	struct Proto *p = checkproto(L, 1);

	lua_newtable(L);
	for (i = 0; i < p->sizep; i++) {
		struct Proto **ip = (struct Proto **)lua_newuserdata(L, sizeof(*ip));
		*ip = p->p[i];
		luaL_getmetatable(L, LFUNC_PROTO);
		lua_setmetatable(L, -2);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int luac__open_proto(lua_State *L)
{
	luaL_Reg lmethods[] = {
		{"line", lua__line},
		{"upvalues", lua__upvalues},
		{"source", lua__source},
		{"nparams", lua__nparams},
		{"is_vararg", lua__is_vararg},
		{"paralist", lua__paralist},
		{"protos", lua__protos},
		{"locals", lua__locals},
		{NULL, NULL},
	};
	luaL_newmetatable(L, LFUNC_PROTO);
	lua_newtable(L);
	luaL_register(L, NULL, lmethods);
	lua_setfield(L, -2, "__index");
	return 0;
}

int luaopen_lfproto(lua_State* L)
{
	luaL_Reg lfuncs[] = {
		{"proto", lua__proto},
		{NULL, NULL},
	};
	luac__open_proto(L);
	luaL_newlib(L, lfuncs);
	return 1;
}

