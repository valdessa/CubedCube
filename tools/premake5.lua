-- Workspace name

workspace "Cubed Cube"	
	location "../build"
    language ("C++")
    cppdialect "C++17"

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

	language "C++"
	kind "Makefile"

    debugcommand("../run.bat")

    -- Comandos para ejecutar el Makefile
    buildcommands { "make -C .." }  -- Cambia a la carpeta anterior y ejecuta make
    rebuildcommands { "make -C .. clean && make -C .." }
    cleancommands { "make -C .. clean" }

    -- Definir las propiedades necesarias para NMake
    buildoutputs { "../bin/%{prj.name}/%{cfg.longname}/*" }  -- Ajusta seg�n tu salida esperada

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
