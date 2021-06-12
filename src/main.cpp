#include "global.hpp"
#include "threading.hpp"
#include <GarrysMod/InterfacePointers.hpp>
#include <steam_api.h>
#include <iostream>
#include <thread>

using namespace GarrysMod::Lua;
using namespace std;

namespace Steamworks {
	struct CSteamworks;
	CSteamGameServerAPIContext* GameServerAPI;
	CSteamworks* SteamworksAPI = nullptr;

	struct CSteamworks {
		STEAM_GAMESERVER_CALLBACK(CSteamworks, OnItemDownloaded, DownloadItemResult_t);
		STEAM_CALLBACK(CSteamworks, OnItemDownloaded2, DownloadItemResult_t);
	};

	void CSteamworks::OnItemDownloaded(DownloadItemResult_t* res)
	{
		cout << "test 1" << endl;
		cout << "AppID:  " << res->m_unAppID << endl;
		cout << "FileID: " << res->m_nPublishedFileId << endl;
		cout << "Result: " << res->m_eResult << endl;
	}

	void CSteamworks::OnItemDownloaded2(DownloadItemResult_t* res)
	{
		cout << "test 2" << endl;
		cout << "AppID:  " << res->m_unAppID << endl;
		cout << "FileID: " << res->m_nPublishedFileId << endl;
		cout << "Result: " << res->m_eResult << endl;
	}

	int DownloadUGC(ILuaBase* LUA)
	{
		ISteamUGC* ugc = GameServerAPI->SteamUGC();
		ISteamUGC* ugc2 = SteamGameServerUGC();
		cout << ugc << ' ' << boolalpha << (ugc == nullptr) << endl;
		cout << ugc2 << ' ' << boolalpha << (ugc2 == nullptr) << endl;

		if (ugc != nullptr) {
			cout << "ugc1" << endl;
			ugc->DownloadItem(2509905428, false);
		}
			

		if (ugc2 != nullptr) {
			cout << "ugc2" << endl;
			ugc2->DownloadItem(2507359662, false);
		}


		return 0;
	}

	void Initialize(ILuaBase* LUA)
	{
		GameServerAPI = InterfacePointers::SteamGameServerAPIContext();
		SteamworksAPI = new CSteamworks;

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