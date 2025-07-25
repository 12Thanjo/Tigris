-- premake5

local vulkan_sdk = os.getenv("VULKAN_SDK")

if(vulkan_sdk == nil) then
	premake.error("Failed to find the Vulkan SDK. Please set the environment variable 'VULKAN_SDK' and try again")
end


project "Vulkan"
	kind "StaticLib"
	-- staticruntime "On"
	

	targetdir(target.lib)
	objdir(target.obj)


	files{
		"./src/**.cpp",
		(vulkan_sdk .. "/include/Volk/volk.c"),
	}

	includedirs{
		"../dependencies/",
		(vulkan_sdk .. "/include/"),
	}


	links{
		"Evo",
	}


project "*"

