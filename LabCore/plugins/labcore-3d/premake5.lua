project "LabCore-3D"
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
		"%{IncludeDir.imgui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.engine}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.pybind11}",
		"%{IncludeDir.cpython}",
		"%{IncludeDir.cpythonPCheaders}",
		"%{IncludeDir.labcore}",
		"%{IncludeDir.vulkanSDK}",

		--"%{IncludeDir.imguizmo}",
		"%{IncludeDir.stb}"
	}

	defines 
	{
		"IMGUI_API=__declspec(dllimport)",
		"PH_SHARED_BUILD",
		"PH_ENGINE_IMPORT", 
		"PH_LABCORE_IMPORT",
		"PH_PLATFORM_IMPORT",
		"YAML_CPP_STATIC_DEFINE"
	}

	links
	{
		"yaml-cpp",
		"assimp",
		"LabCore",
		"%{Library.platform}"
	}

	libdirs {
		"%{LibraryDir.cpython}"
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
			"%{Library.shaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Tools_Debug}"
		}



	filter "configurations:Release"
		defines "PH_RELEASE"
		runtime "Release"
		optimize "on"

		links {
			"%{Library.shaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Tools_Release}"
		}


	filter "configurations:Dist"
		defines "PH_DIST"
		runtime "Release"
		optimize "on"

		links {
			"%{Library.shaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Tools_Release}"
		}

    