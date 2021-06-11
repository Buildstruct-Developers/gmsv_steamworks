#include "global.hpp"
#include <GarrysMod/InterfacePointers.hpp>
#include <steam_api.h>
#include <lua.hpp>
#include <iostream>
#include <thread>

using namespace GarrysMod::Lua;
using namespace std;

namespace Steamworks {
	CSteamGameServerAPIContext* GameServerAPI;

	int DownloadUGC(ILuaBase* LUA)
	{

		return 0;
	}

	void Initialize(ILuaBase* LUA)
	{
		CSteamGameServerAPIContext* api = InterfacePointers::SteamGameServerAPIContext();

		LUA->CreateTable();
			SET_FUNC(DownloadUGC);
		LUA->SetField(INDEX_GLOBAL, "steamworks");
	}

	void Deinitialize(ILuaBase* LUA)
	{

	}
}

// GMOD ENTRY POINT
GMOD_MODULE_OPEN()
{
	Steamworks::Initialize(LUA);
	return 0;
}

GMOD_MODULE_CLOSE()
{
	Steamworks::Deinitialize(LUA);
	return 0;
}