#include "threading.hpp"
#include <stdlib.h>
#include <iostream>

using namespace Threading;
using namespace GarrysMod::Lua;
using namespace std;

// Utils
char Utils::RandomChar()
{
	return 'A' + rand() % 26;
}

void Utils::RandomChars(string& str, int size)
{
	for (int i = 0; i < size; i++) {
		str += RandomChar();
	}
}

// Core
int Core::ThinkHandler(lua_State* LUA)
{
	lock_guard<mutex> queue_lock(globalLock);
	while (!lockQueue.empty()) {
		Lock& l = lockQueue.front();

		{
			lock_guard<mutex> lck(l.m);
			l.step1 = true;
		}
		l.cv.notify_one();

		// wait for thread
		{
			unique_lock<mutex> lck(l.m);
			l.cv.wait(lck, [&l]{ return l.step2; });
		}

		lockQueue.pop();
	}

	return 0;
}

Core::Lock* Core::CreateLock()
{
	lock_guard<mutex> lck(globalLock);
	lockQueue.emplace();
	return &lockQueue.back();
}

void Core::Sync(Core::Lock* l)
{
	unique_lock<mutex> lck(l->m);
	l->cv.wait(lck, [&l] { return l->step1; });
}

void Core::Desync(Core::Lock* l)
{
	{
		lock_guard<mutex> lck(l->m);
		l->step2 = true;
	}

	l->cv.notify_one();
}

Core::LuaThread Core::Alloc(lua_State* L)
{
	LuaThread lt;
	lt.state = lua_newthread(L);
	lt.ref = luaL_ref(L, LUA_REGISTRYINDEX);
	return lt;
}

void Core::Dealloc(LuaThread& lt)
{
	luaL_unref(lt.state, LUA_REGISTRYINDEX, lt.ref);
}

void Core::Initialize(ILuaBase* LUA)
{
	if (!uniqueID.empty())
		Deinitialize(LUA);

	uniqueID = "_THREADING_";
	Utils::RandomChars(uniqueID, 4);

	LUA->GetField(INDEX_GLOBAL, "timer");
		LUA->GetField(-1, "Create");
		LUA->PushString(uniqueID.c_str());
		LUA->PushNumber(0);
		LUA->PushNumber(0);
		LUA->PushCFunction(ThinkHandler);
		LUA->Call(4, 0);
	LUA->Pop();

	Thread::Initialize(LUA);
}

void Core::Deinitialize(ILuaBase* LUA)
{
	LUA->GetField(INDEX_GLOBAL, "timer");
		LUA->GetField(-1, "Remove");
		LUA->PushString(uniqueID.c_str());
		LUA->Call(1, 0);
	LUA->Pop();

	uniqueID.clear();

	Thread::Deinitialize(LUA);
}

// Thread
int Thread::META;

int Thread::__gc(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Thread** tp = (Thread**)(lua_touserdata(L, 1));
	if (!tp)
		return 0;
	Thread* t = *tp;

	if (t->destroy)
		delete t;
	else
		t->destroy = true;

	return 0;
}

int Thread::wait(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Thread** tp = (Thread**)(lua_touserdata(L, 1));
	if (!tp)
		return 0;
	Thread* t = *tp;

	if (t->joined)
		return 0;

	if (t->thr.joinable()) {
		t->joined = true;

		if (t->lastLock) {
			lock_guard<mutex> lck(t->lastLock->m);
			t->lastLock->step2 = true;

			if (!t->lastLock->step1) {
				t->lastLock->step1 = true;
				t->lastLock->cv.notify_one();
			}
		}

		t->thr.join();
	}

	return 0;
}

Thread::~Thread()
{
	if (lastLock)
		Desync();

	if (thr.joinable())
		thr.detach();
}

void Thread::New(lua_State* L, Thread* t)
{
	*(Thread**)lua_newuserdata(L, sizeof(Thread*)) = t;
	lua_rawgeti(L, LUA_REGISTRYINDEX, META);
	lua_setmetatable(L, -2);
}

void Thread::Sync()
{
	if (lastLock)
		Desync();

	if (joined)
		return;

	lastLock = Core::CreateLock();
	if (joined)
		return;

	Core::Sync(lastLock);
}

void Thread::Desync()
{
	if (!lastLock)
		return;

	Core::Desync(lastLock);
	lastLock = nullptr;
}

void Thread::Initialize(ILuaBase* LUA)
{
	lua_State* L = LUA->GetState();

	static const luaL_Reg lib_funcs[] = {
		{"__gc", __gc},
		{"wait", wait},
		{NULL, NULL}
	};

	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushstring(L, "CThread");
	lua_setfield(L, -2, "MetaName");

	luaL_setfuncs(L, lib_funcs, 0);
	META = luaL_ref(L, LUA_REGISTRYINDEX);
}

void Thread::Deinitialize(ILuaBase* LUA)
{
	lua_State* L = LUA->GetState();

	luaL_unref(L, LUA_REGISTRYINDEX, META);
	META = LUA_REFNIL;
}