-- Workspace name

workspace "Cubed Cube"	
	location "../build"
    language ("C++")
    cppdialect "C++20"

    architecture "x86_64"
    
    configurations { "Debug" , "Release" }

    symbolspath '$(OutDir)vc$(PlatformToolsetVersion).pdb'

    filter "configurations:Debug"
        targetsuffix "_debug"
        defines { "DEBUG"}
		runtime "Debug"
		symbols "Full"
        ignoredefaultlibraries { "libcmt" }

	filter "configurations:Release"
        targetsuffix "_release"
        defines { "NDEBUG"}
		runtime "Release"
        symbols "Off"
		optimize "On"

 	filter { }

    filter "system:windows"
		systemversion "latest"
		staticruntime "On"

    defines { "_CRT_SECURE_NO_WARNINGS","GLEW_STATIC", "NOMINMAX" }

project "Cubed Cube"

	kind "ConsoleApp"
	language "C++"

 	files { "../include/**.h", "../source/**.cpp" }

    targetdir ("../bin/%{prj.name}/%{cfg.longname}")
 	objdir ("../bin/obj/%{prj.name}/%{cfg.longname}")

    includedirs {
        "../build",
        "../examples",
        "../include",
	    "../deps",
        "../deps/glm",
        "../deps/GRRLIB",
        "../deps/libogc",
    }
    
    libdirs {
        "../deps/GRRLIB/",
    }

    -- Link libs based on configuration
    filter "configurations:Debug"
        links {
            "GRRLIB",
        }

    filter "configurations:Release"
        links {
            "GRRLIB",
        }