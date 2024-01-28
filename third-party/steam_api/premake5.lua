local current_dir = _SCRIPT_DIR

function IncludeSteamAPI()

    filter {'system:windows', 'architecture:x86'}
        libdirs(current_dir .. '/bin/')
        
    filter {"system:windows", "architecture:x86_64"}
        libdirs(current_dir .. '/bin/win64')

    filter {"system:linux", "architecture:x86"}
        libdirs(path.getabsolute(current_dir) .. '/bin/linux32')

    filter {"system:linux", "architecture:x86_64"}
        libdirs(path.getabsolute(current_dir) .. '/bin/linux64')
    
    filter {}

    externalincludedirs{current_dir .. '/include'}
    links 'steam_api'
end