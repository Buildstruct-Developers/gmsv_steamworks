#include "global.hpp"

using namespace GarrysMod::Lua;
using namespace std;

namespace Steamworks {
	void Initialize(ILuaBase* LUA)
	{
		LUA->GetField(INDEX_GLOBAL, "print");
		LUA->PushString("Hello Steamworks!");
		LUA->Call(1, 0);
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