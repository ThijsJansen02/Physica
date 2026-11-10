project "LabCore-ssh"
	kind "SharedLib"
	language "C++"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/int")

	--ignoredefaultlibraries { "libraries" }
	staticruntime "Off"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"res/**"
	}

	removefiles 
	{
		"res/plugins/source/**"
	}

	includedirs
	{
		"%{IncludeDir.platform}",
		"%{IncludeDir.base}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.engine}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.pybind11}",
		"%{IncludeDir.cpython}",
		"%{IncludeDir.cpythonPCheaders}",
		"%{IncludeDir.labcore}", 
		"%{IncludeDir.libssh}",
		"%{IncludeDir.imgui}"
	}

	defines 
	{
		"IMGUI_API=__declspec(dllimport)",
		"PH_SHARED_BUILD",
		"PH_ENGINE_IMPORT", 
		"PH_LABCORE_IMPORT",
		"PH_PLATFORM_IMPORT",
		"LC_SSH_EXPORT",
		"YAML_CPP_STATIC_DEFINE",
		"LC_PLUGIN_PYTHON"
	}

	links
	{
		"yaml-cpp",
		"LabCore",
		"Ws2_32.lib",
		"Iphlpapi.lib",
		"Crypt32.lib",
		"%{Library.platform}"
	}

	libdirs {
		"%{LibraryDir.cpython}",
		"%{LibraryDir.openssl}",
		"%{LibraryDir.libssh}"
	}

	postbuildcommands 
	{
		--"copy /b /y 'P:\\My Documents\\BRP\\GUI\\ProjectPhysica\\bin\\Debug-windows-x86_64\\RP-GUI\\RP-GUI.dll' 'P:\\My Documents\\BRP\\GUI\\ProjectPhysica\\bin\\Debug-windows-x86_64\\Platform'"
		"{COPYFILE} \"%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/%{cfg.buildtarget.name}\" \"%{wks.location}/bin/" .. outputdir .. "/Platform\""
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"PH_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "PH_DEBUG"
		runtime "Debug"
		symbols "on"

		
		links {
			"%{Library.openssl_debug}",
			"%{Library.libssh_debug}",
			"%{Library.libcrypto_debug}",
			"%{Library.zlib_debug}"
		}



	filter "configurations:Release"
		defines "PH_RELEASE"
		runtime "Release"
		optimize "on"

		links {
			"%{Library.openssl}",
			"%{Library.libssh}",
			"%{Library.libcrypto}",
			"%{Library.zlib}"
		}


	filter "configurations:Dist"
		defines "PH_DIST"
		runtime "Release"
		optimize "on"

		links {
			"%{Library.openssl}",
			"%{Library.libssh}",
			"%{Library.libcrypto}",
			"%{Library.zlib}"
		}

    