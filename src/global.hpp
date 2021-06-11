#ifndef STEAMWORKS_GLOBAL_HPP
#define STEAMWORKS_GLOBAL_HPP

#include <GarrysMod/Lua/Interface.h>

#define PUSH_FUNC(FUNC) \
	LUA->PushCFunction([](lua_State* L) -> int { \
		GarrysMod::Lua::ILuaBase* LUA = L->luabase; \
		LUA->SetState(L); \
		return FUNC(LUA); \
	})

#endif