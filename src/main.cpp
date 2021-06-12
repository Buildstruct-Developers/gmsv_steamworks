#include "global.hpp"
#include "threading.hpp"
#include <steam_api.h>
#include <iostream>
#include <list>
#include <algorithm>
#include <filesystem>

using namespace GarrysMod::Lua;
using namespace std;

namespace Steamworks {
	struct CSteamworks {
		struct AddonFile {
			Threading::Core::LuaThread lt;
			PublishedFileId_t id;
			int callback;
		};

		list<AddonFile> AddonQueue;
		void PushCallback(lua_State* L, PublishedFileId_t id, int ref);

		STEAM_GAMESERVER_CALLBACK(CSteamworks, OnItemDownloaded, DownloadItemResult_t);
	};
	CSteamworks* SteamworksAPI = nullptr;

	void CSteamworks::PushCallback(lua_State* L, PublishedFileId_t id, int ref)
	{
		Threading::Core::LuaThread lt = Threading::Core::Alloc(L);
		AddonFile af{ lt, id, ref };
		AddonQueue.push_back(af);
	}

	void CSteamworks::OnItemDownloaded(DownloadItemResult_t* res)
	{
		if (res->m_unAppID != 4000)
			return;

		bool exists = find_if(AddonQueue.begin(), AddonQueue.end(), [&res](AddonFile& af) {
			return af.id == res->m_nPublishedFileId;
		}) != AddonQueue.end();

		if (!exists)
			return;

		string path;
		bool ok = res->m_eResult == k_EResultOK;
		if (ok) {
			uint64 punSizeOnDisk;
			uint32 punTimeStamp;
			char* pchFolder = new char[256];

			ok = SteamGameServerUGC()->GetItemInstallInfo(res->m_nPublishedFileId, &punSizeOnDisk, pchFolder, 256, &punTimeStamp);
			if (ok) {
				auto it = filesystem::directory_iterator(pchFolder);
				if (it == filesystem::end(it))
					ok = false;
				else
					path = it->path().string();
			}

			delete[] pchFolder;
		}

		AddonQueue.remove_if([&](AddonFile& af) {
			if (af.id != res->m_nPublishedFileId)
				return false;

			lua_State* L = af.lt.state;
			lua_rawgeti(L, LUA_REGISTRYINDEX, af.callback);
			ok ? lua_pushstring(L, path.c_str()) : lua_pushnil(L);
			if (lua_pcall(L, 1, 0, 0) != 0) {
				luaL_traceback(L, L, lua_tostring(L, -1), 0);
				cout << lua_tostring(L, -1) << endl;
				lua_pop(L, 2);
			}

			luaL_unref(L, LUA_REGISTRYINDEX, af.callback);
			Threading::Core::Dealloc(af.lt);
			return true;
		});
	}

	int DownloadUGC(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		const char* id_str = LUA->CheckString(1);
		LUA->CheckType(2, Type::Function);

		bool success = false;
		PublishedFileId_t id = std::stoull(id_str);
		ISteamUGC* ugc = SteamGameServerUGC();
		if (ugc)
			success = SteamGameServerUGC()->DownloadItem(id, false);

		if (success) {
			lua_pushvalue(L, 2);
			int ref = luaL_ref(L, LUA_REGISTRYINDEX);

			SteamworksAPI->PushCallback(L, id, ref);
		}

		LUA->PushBool(success);
		return 1;
	}

	void Initialize(ILuaBase* LUA)
	{
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