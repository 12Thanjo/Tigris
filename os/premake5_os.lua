
project "os"
	kind "StaticLib"
	-- staticruntime "On"
	

	targetdir(target.lib)
	objdir(target.obj)


	filter "system:Windows"
		files{
			"./Windows/src/**.cpp",
		}
	filter {}

	includedirs{
		"../dependencies/",
	}


	links{
		"Evo",
	}


project "*"

