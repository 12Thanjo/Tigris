
project "tigris"
	kind "ConsoleApp"
	-- staticruntime "On"
	

	targetdir(target.bin)
	objdir(target.obj)

	files{
		"./src/**.cpp",
	}

	includedirs{
		"./include/",
		"../dependencies/",
		"../Vulkan/include/"
	}


	links{
		"Evo",
		"os",
		"Vulkan",
	}


project "*"

