#ifndef STEAMWORKS_GLOBAL_HPP
#define STEAMWORKS_GLOBAL_HPP

#include <GarrysMod/Lua/Interface.h>

#define PUSH_FUNC(FUNC) \
	LUA->PushCFunction(FUNC)

#define SET_FUNC(FUNC) \
	PUSH_FUNC(FUNC); \
	LUA->SetField(-2, #FUNC) \

#endif