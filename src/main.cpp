#include "global.hpp"
#include <GarrysMod/InterfacePointers.hpp>
#include <steam_api.h>
#include <iostream>

using namespace GarrysMod::Lua;
using namespace std;

namespace Steamworks {
	CSteamGameServerAPIContext* GameServerAPI;

	int Test(ILuaBase* LUA)
	{
		cout << "Hello World!" << endl;
		return 0;
	}

	void Initialize(ILuaBase* LUA)
	{
		CSteamGameServerAPIContext* api = InterfacePointers::SteamGameServerAPIContext();

		PUSH_FUNC(Test);
		LUA->SetField(INDEX_GLOBAL, "Test");
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