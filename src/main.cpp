#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/InterfacePointers.hpp>
#include "LuaThreading.hpp"

#include <steam_api.h>
#include <string>
#include <list>
#include <filesystem.h>
#include <tinydir.h>

int LuaErrorHandler(lua_State* L)
{
	luaL_traceback(L, L, lua_tostring(L, 1), 1);
	printf("%s\n", lua_tostring(L, -1));
	lua_pop(L, 1);

	return 0;
}

class CSteamWorks {
public:
	struct SteamCallback {
		Retro::LuaThreading::LuaState state;
		PublishedFileId_t id;
		int cb;
	};

	bool RequestUGCDetails(PublishedFileId_t id);

	static std::list<SteamCallback> DownloadUGCQueue;
	static std::list<SteamCallback> FileInfoQueue;
	static CSteamWorks* Singleton;

private:
	STEAM_GAMESERVER_CALLBACK(CSteamWorks, OnItemDownloaded, DownloadItemResult_t);

	void OnGetUGCDetails(SteamUGCRequestUGCDetailsResult_t* res, bool bIOFailure);
	CCallResult<CSteamWorks, SteamUGCRequestUGCDetailsResult_t> m_UGCDetailsCallResult;
};

std::list<CSteamWorks::SteamCallback> CSteamWorks::DownloadUGCQueue;
std::list<CSteamWorks::SteamCallback> CSteamWorks::FileInfoQueue;
CSteamWorks* CSteamWorks::Singleton;

bool CSteamWorks::RequestUGCDetails(PublishedFileId_t id)
{
	ISteamUGC* ugc = SteamGameServerUGC();
	if (!ugc)
		return false;

	SteamAPICall_t h = ugc->RequestUGCDetails(id, 0);
	if (h == k_uAPICallInvalid)
		return false;

	CSteamWorks::Singleton->m_UGCDetailsCallResult.Set(h, this, &CSteamWorks::OnGetUGCDetails);
	return true;
}

void CSteamWorks::OnItemDownloaded(DownloadItemResult_t* res)
{
	if (res->m_unAppID != 4000)
		return;

	auto it = std::find_if(DownloadUGCQueue.begin(), DownloadUGCQueue.end(), [&res](SteamCallback& sc) {
		return res->m_nPublishedFileId == sc.id;
	});

	if (it == DownloadUGCQueue.end())
		return;

	SteamCallback& sc = *it;
	lua_State* L = sc.state.get();

	std::string path;
	bool success = res->m_eResult == k_EResultOK;
	if (success) {
		uint64 punSizeOnDisk;
		uint32 punTimeStamp;
		char* pchFolder = new char[256];

		success = SteamGameServerUGC()->GetItemInstallInfo(res->m_nPublishedFileId, &punSizeOnDisk, pchFolder, 256, &punTimeStamp);
		if (success) {
			if (g_pFullFileSystem->IsDirectory(pchFolder)) {
				tinydir_dir* dir = new tinydir_dir;
				tinydir_file* file = new tinydir_file;

				tinydir_open(dir, pchFolder);

				while (dir->has_next) {
					tinydir_readfile(dir, file);
					
					if (!file->is_dir) {
						V_FixSlashes(file->path);
						path = file->path;
						break;
					}

					tinydir_next(dir);
				}

				tinydir_close(dir);

				delete dir;
				delete file;
			} else {
				path = pchFolder;
			}
			
		}

		delete[] pchFolder;
	}

	lua_pushcfunction(L, LuaErrorHandler);
	lua_rawgeti(L, LUA_REGISTRYINDEX, sc.cb);
	success ? lua_pushstring(L, path.c_str()) : lua_pushnil(L);
	if (lua_pcall(L, 1, 0, -3) != 0)
		lua_pop(L, 1);
	lua_pop(L, 1);

	luaL_unref(L, LUA_REGISTRYINDEX, sc.cb);
	DownloadUGCQueue.erase(it);
}

void CSteamWorks::OnGetUGCDetails(SteamUGCRequestUGCDetailsResult_t* res, bool bIOFailure)
{
	auto it = std::find_if(FileInfoQueue.begin(), FileInfoQueue.end(), [&res](SteamCallback& sc) {
		return res->m_details.m_nPublishedFileId == sc.id;
	});

	if (it == FileInfoQueue.end())
		return;

	SteamCallback& sc = *it;
	lua_State* L = sc.state.get();
	bool success = res->m_details.m_eResult == k_EResultOK;
	if (success) {
		lua_createtable(L, 0, 0);
		lua_pushstring(L, std::to_string(res->m_details.m_nPublishedFileId).c_str()); lua_setfield(L, -2, "id");
		lua_pushstring(L, res->m_details.m_rgchTitle); lua_setfield(L, -2, "title");
		lua_pushstring(L, res->m_details.m_rgchDescription); lua_setfield(L, -2, "description");
		lua_pushstring(L, std::to_string(res->m_details.m_hFile).c_str()); lua_setfield(L, -2, "fileid");
		lua_pushstring(L, std::to_string(res->m_details.m_hPreviewFile).c_str()); lua_setfield(L, -2, "previewid");
		//lua_pushnumber(L, res->m_details.m_nPublishedFileId); lua_setfield(L, -2, "previewurl");
		lua_pushstring(L, std::to_string(res->m_details.m_ulSteamIDOwner).c_str()); lua_setfield(L, -2, "owner");
		lua_pushinteger(L, res->m_details.m_rtimeCreated); lua_setfield(L, -2, "created");
		lua_pushinteger(L, res->m_details.m_rtimeUpdated); lua_setfield(L, -2, "updated");
		lua_pushboolean(L, res->m_details.m_bBanned); lua_setfield(L, -2, "banned");
		lua_pushstring(L, res->m_details.m_rgchTags); lua_setfield(L, -2, "tags");
		lua_pushinteger(L, res->m_details.m_nFileSize); lua_setfield(L, -2, "size");
		lua_pushinteger(L, res->m_details.m_nPreviewFileSize); lua_setfield(L, -2, "previewsize");
		lua_pushnumber(L, 0); lua_setfield(L, -2, "error");
		lua_newtable(L); lua_setfield(L, -2, "children");
		lua_pushinteger(L, res->m_details.m_unVotesDown); lua_setfield(L, -2, "down");
		lua_pushinteger(L, res->m_details.m_unVotesUp); lua_setfield(L, -2, "up");
		lua_pushinteger(L, res->m_details.m_unVotesDown + res->m_details.m_unVotesUp); lua_setfield(L, -2, "total");
		lua_pushnumber(L, res->m_details.m_flScore); lua_setfield(L, -2, "score");
		lua_pushinteger(L, res->m_details.m_eVisibility); lua_setfield(L, -2, "visibility");
	}

	lua_pushcfunction(L, LuaErrorHandler);
	lua_rawgeti(L, LUA_REGISTRYINDEX, sc.cb);
	success ? lua_pushvalue(L, -3) : lua_pushnil(L);
	if (lua_pcall(L, 1, 0, -3) != 0)
		lua_pop(L, 1);
	lua_pop(L, success ? 2 : 1);

	luaL_unref(L, LUA_REGISTRYINDEX, sc.cb);
	FileInfoQueue.erase(it);
}

int DownloadUGC(lua_State* L)
{
	GarrysMod::Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	PublishedFileId_t id = std::stoull(LUA->CheckString(1));
	LUA->CheckType(2, GarrysMod::Lua::Type::Function);

	bool success = false;
	ISteamUGC* ugc = SteamGameServerUGC();
	if (ugc)
		success = ugc->DownloadItem(id, false);

	if (success) {
		lua_pushvalue(L, 2);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		
		CSteamWorks::DownloadUGCQueue.push_back({
			Retro::LuaThreading::CreateState(L),
			id,
			ref
		});
	} else {
		LUA->PushCFunction(LuaErrorHandler);
		LUA->Push(2);
		LUA->PCall(0, 0, -2);
		LUA->Pop();
	}

	LUA->PushBool(success);
	return 1;
}

int FileInfo(lua_State* L)
{
	GarrysMod::Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	PublishedFileId_t id = std::stoull(LUA->CheckString(1));
	LUA->CheckType(2, GarrysMod::Lua::Type::Function);

	bool success = CSteamWorks::Singleton->RequestUGCDetails(id);

	if (success) {
		lua_pushvalue(L, 2);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		CSteamWorks::FileInfoQueue.push_back({
			Retro::LuaThreading::CreateState(L),
			id,
			ref
		});
	}
	else {
		LUA->PushCFunction(LuaErrorHandler);
		LUA->Push(2);
		LUA->PCall(0, 0, -2);
		LUA->Pop();
	}

	LUA->PushBool(success);
	return 1;
}

int MountLegacy(lua_State* L)
{
	GarrysMod::Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	std::string path = LUA->CheckString(1);

	bool exists = g_pFullFileSystem->FileExists(path.c_str());
	if (!exists) 
		{ LUA->PushBool(false); return 1; }


	return 0;
}

GMOD_MODULE_OPEN()
{
	CSteamWorks::Singleton = new CSteamWorks;
	g_pFullFileSystem = InterfacePointers::FileSystem();
	if (!g_pFullFileSystem)
		LUA->ThrowError("[Steamworks] Filesystem got fucked");

	LUA->CreateTable();
		LUA->PushCFunction(DownloadUGC); LUA->SetField(-2, "DownloadUGC");
		LUA->PushCFunction(FileInfo); LUA->SetField(-2, "FileInfo");
	LUA->SetField(GarrysMod::Lua::INDEX_GLOBAL, "steamworks");

	LUA->GetField(GarrysMod::Lua::INDEX_GLOBAL, "hook");
		LUA->GetField(-1, "Add");
		LUA->PushString("Think");
		LUA->PushString("_STEAMWORKS_THINK");
		LUA->PushCFunction([](lua_State* L) { Retro::LuaThreading::Think(); return 0; });
		LUA->Call(3, 0);
	LUA->Pop();

	return 1;
}
GMOD_MODULE_CLOSE()
{
	delete CSteamWorks::Singleton;

	return 0;
}