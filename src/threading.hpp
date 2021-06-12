#ifndef THREADING_HPP
#define THREADING_HPP
#include <GarrysMod/Lua/Interface.h>
#include <lua.hpp>
#include <queue>
#include <condition_variable>
#include <string>
#include <thread>

namespace Threading {
	namespace Utils {
		char RandomChar();
		void RandomChars(std::string& str, int size);
	}

	namespace Core {
		struct Lock {
			std::mutex m;
			std::condition_variable cv;
			bool step1 = false;
			bool step2 = false;
		};

		struct LuaThread {
			lua_State* state;
			int ref;
		};

		typedef std::queue<Lock> LockQueue;
		static LockQueue lockQueue;
		static std::mutex globalLock;
		static std::string uniqueID;

		int	ThinkHandler(lua_State* L);


		Lock* CreateLock();
		void Sync(Lock* l);
		void Desync(Lock* l);

		LuaThread Alloc(lua_State* L);
		void Dealloc(LuaThread& lt);

		void Initialize(GarrysMod::Lua::ILuaBase* LUA);
		void Deinitialize(GarrysMod::Lua::ILuaBase* LUA);
	}

	class Thread {
		std::thread thr;
		Core::Lock* lastLock = nullptr;

		static int __gc(lua_State* L);
		static int wait(lua_State* L);
	public:
		static int META;

		std::atomic_bool destroy = false;
		std::atomic_bool joined = false;
		~Thread();

		static void New(lua_State* L, Thread* t);
		template<class Func>
		static void Create(lua_State* L, Func&& f)
		{
			Core::LuaThread lt = Core::Alloc(L);

			Thread* t = new Thread;
			t->thr = std::thread([](Core::LuaThread lt, Thread* t, Func&& f) {
				f(lt.state, t);
				Core::Dealloc(lt);

				if (t->destroy)
					delete t;
				else
					t->destroy = true;
			}, lt, t, f);

			New(L, t);
		}

		void Sync();
		void Desync();

		static void Initialize(GarrysMod::Lua::ILuaBase* LUA);
		static void Deinitialize(GarrysMod::Lua::ILuaBase* LUA);
	};
}

#endif