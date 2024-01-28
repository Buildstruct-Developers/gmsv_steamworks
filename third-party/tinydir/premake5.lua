local current_dir = _SCRIPT_DIR

function IncludeTinydir()
	print(current_dir)
	externalincludedirs(current_dir)
end