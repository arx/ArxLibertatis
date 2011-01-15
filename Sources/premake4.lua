solution "ArxFatalis"
	configurations { "Debug", "Release" }
	location "build"

	defines { "_UNICODE" }
	flags { "NoExceptions", "NoRTTI", "Unicode" }

	configuration "Debug"
		defines { "_DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "OptimizeSpeed" }

	configuration { "windows" }
		defines { "WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }

	project "ArxCommon"
		kind "StaticLib"
		language "C++"
		defines { "_LIB" }
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"ArxCommon/*.cpp",
		}

	project "Athena"
		kind "SharedLib"
		language "C++"
		defines { "_USRDLL", "ATHENA_EXPORTS", "AAL_APIDLL" }
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"Athena/*.cpp",
			"Athena/*.h",
		}
		links
		{
			"ArxCommon",
			"HERMES",
			"ARX_SCRIPT_DEBUGGER",
		}

	project "EErie"
		kind "StaticLib"
		language "C++"
		defines { "_LIB" }
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"EERIE/*.cpp",
		}

	project "HERMES"
		kind "StaticLib"
		language "C++"
		defines { "_LIB" }
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"HERMES/*.cpp",
			"HERMES/HERMESnet.rc",
		}

	project "Mercury"
		kind "StaticLib"
		language "C++"
		defines { "_LIB" }
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"Mercury/*.cpp",
		}

	project "MINOS"
		kind "StaticLib"
		language "C++"
		defines { "_LIB" }
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"MINOS/*.cpp",
		}

	project "DANAE"
		kind "WindowedApp"
		language "C++"
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"DANAE/*.cpp",
			"DANAE/*.h",
			"DANAE/*.rc",
		}
		links
		{
			"ArxCommon",
			"Athena",
			"EErie",
			"HERMES",
			"Mercury",
			"MINOS",
			"ARX_SCRIPT_DEBUGGER",
		}

	project "ARX_SCRIPT_DEBUGGER"
		kind "SharedLib"
		language "C++"
		defines { "_USRDLL", "ARX_SCRIPT_DEBUGGER_EXPORTS", "APIDLL" }
		includedirs
		{
			"Include/OtherLibs",
			"Include",
		}
		files
		{
			"DANAE_Debugger/resource.h",
			--"DANAE_Debugger/SCRIPT_DEBUGGER.def",
			"DANAE_Debugger/SCRIPT_DEBUGGER.rc",
			"DANAE_Debugger/SCRIPT_DEBUGGER_Dialog.cpp",
			"DANAE_Debugger/SCRIPT_DEBUGGER_Dialog.h",
		}
