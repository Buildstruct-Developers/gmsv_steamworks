#include "global.hpp"
#include "threading.hpp"
#include <GarrysMod/InterfacePointers.hpp>
#include <steam_api.h>
#include <iostream>
#include <thread>

using namespace GarrysMod::Lua;
using namespace std;

namespace Steamworks {
	CSteamGameServerAPIContext* GameServerAPI;

	int DownloadUGC(ILuaBase* LUA)
	{
		Threading::Thread::Create(LUA->GetState(), [](lua_State* L, Threading::Thread* t) {
			cout << "Thread start" << endl;

			t->Sync();
			cout << "This message is synced!" << endl;
			t->Desync();

			cout << "Now i'm desynced!" << endl;
		});

		return 1;
	}

	void Initialize(ILuaBase* LUA)
	{
		CSteamGameServerAPIContext* api = InterfacePointers::SteamGameServerAPIContext();

		Threading::Core::Initialize(LUA);

		LUA->CreateTable();
			SET_FUNC(DownloadUGC);
		LUA->SetField(INDEX_GLOBAL, "steamworks");
	}

	void Deinitialize(ILuaBase* LUA)
	{
		Threading::Core::Deinitialize(LUA);
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