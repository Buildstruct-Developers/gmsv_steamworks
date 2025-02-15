PROJECT_GENERATOR_VERSION = 3

newoption({
    trigger = "gmcommon",
    description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
    value = "path to garrysmod_common directory"
})
local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
    "you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory")
include(gmcommon)

include "third-party/steam_api" -- Replaces IncludeSteamAPI
include "third-party/tinydir"
include "third-party/LuaThreading"


CreateWorkspace({ name = "steamworks", abi_compatible = false })
    CreateProject({ serverside = true, manual_files = true })
        IncludeHelpersExtended()
        IncludeLuaShared()
        IncludeSteamAPI()
        IncludeSDKCommon()
        IncludeSDKTier0()
        IncludeSDKTier1()
        IncludeSDKMathlib()

        IncludeTinydir()
        IncludeLuaThreading()


        files {"src/*.cpp", "src/*.hpp"}
        externalincludedirs { "third-party/tinydir" }
        externalincludedirs { "third-party/pocketlzma" }