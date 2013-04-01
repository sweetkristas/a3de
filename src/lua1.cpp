#include <iostream>

#include "lua.hpp"

#include "stdafx.h"

/* the Lua interpreter */
lua_State* L;

static int average(lua_State *L)
{
	// get number of arguments
	int n = lua_gettop(L);
	double sum = 0.0;
	for(int i=1; i <= n; ++i) {
		if(!lua_isnumber(L, i)) {
			lua_pushstring(L, "Incorrect argument to 'average'");
			lua_error(L);
		}
		sum += lua_tonumber(L, i);
	}
	lua_pushnumber(L, sum / n);
	lua_pushnumber(L, sum);
	return 2;
}

int main(int argc, char *argv[])
{
	/* initialize Lua */
	L = luaL_newstate();

	/* load various Lua libraries */
	luaL_openlibs(L);
	lua_register(L, "average", average);
	int ret = luaL_loadfile(L, "run1.lua");
	if(ret != LUA_OK) {
		std::cerr << "Error loading file run1.lua: " << ret << std::endl;
	} else {
		lua_pcall(L, 0, 0, 0);
	}
	
	/* cleanup Lua */
	lua_close(L);

	return 0;
}

