-- premake5

workspace "Tigris"
	architecture "x64"

	configurations{
		"Debug",
		"Release",
	}

	platforms {
		"Windows",
		"Linux",
	}
	defaultplatform "Windows"


	flags{
		"MultiProcessorCompile",
	}


	startproject "pthr"


	---------------------------------------
	-- platform

	filter "system:Windows"
		system "windows"
	filter {}


	filter "system:Linux"
		system "linux"
	filter {}



	------------------------------------------------------------------------------
	-- configs

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
		optimize "Off"

		defines{
			"_DEBUG",
		}
	filter {}


	filter "configurations:Release"
		runtime "Release"
		symbols "Off"
		optimize "Full"

		defines{
			"NDEBUG",
		}

		linktimeoptimization "On"
	filter {}



------------------------------------------------------------------------------
-- global variables
	
config = {
	location = ("%{wks.location}"),
	platform = ("%{cfg.platform}"),
	build    = ("%{cfg.buildcfg}"),
	project  = ("%{prj.name}"),
}


target = {
	bin = string.format("%s/build/%s/%s/bin/",   config.location, config.platform, config.build),
	lib = string.format("%s/build/%s/%s/lib/%s", config.location, config.platform, config.build, config.project),
	obj = string.format("%s/build/%s/%s/obj/%s", config.location, config.platform, config.build, config.project),
}



------------------------------------------------------------------------------
-- extern lib projects

include "./dependencies/premake5_Evo.lua"


------------------------------------------------------------------------------
-- project settings

language "C++"
cppdialect "C++23"
exceptionhandling "Off"
rtti "Off"
allmodulespublic "On"


------------------------------------------------------------------------------
-- projects

include "./os/premake5_os.lua"
include "./Vulkan/premake5_vulkan.lua"
include "./Tigris/premake5_tigris.lua"


---------------------------------------
-- build

filter "configurations:Debug"
	warnings "High"
filter {}


filter "configurations:Release"
	
filter {}
