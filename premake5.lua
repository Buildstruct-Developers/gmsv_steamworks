PROJECT_GENERATOR_VERSION = 3

newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
	value = "path to garrysmod_common directory"
})

gmcommon = _OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON")
if not gmcommon then
    print("you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory. Using built in instead.")
end
include(gmcommon or 'third-party/garrysmod_common')

CreateWorkspace({ name = "steamworks", abi_compatible = false })
    CreateProject({ serverside = true, manual_files = true })
        IncludeHelpersExtended()
        IncludeLuaShared()
        IncludeSDKCommon()
        IncludeSteamAPI()
        IncludeSDKTier0()
		IncludeSDKTier1()

        files {"src/*.cpp", "src/*.hpp"}