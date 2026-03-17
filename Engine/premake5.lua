project "Engine"
	kind "StaticLib"
	language "C++"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/int")
	staticruntime "off"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"res/**"
	}

	includedirs
	{
		"%{IncludeDir.platform}",
        "%{IncludeDir.base}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.dep}",
		"%{IncludeDir.assimp}",
		"%{prj.location}/dep/include",
		"%{IncludeDir.vulkanSDK}",
		"%{IncludeDir.imguizmo}",
		"%{IncludeDir.engine}"
	}

	
	libdirs
	{
		"%{wks.location}/dep/bin/win32-release/assimp",
		"%{LibraryDir.vulkanSDK}"
	}


	links 
	{
		"Base",
		"Imgui",
		"yaml-cpp",
		"assimp",
		"ImGuizmo"
	}


	defines 
	{
		"PH_SCENE_IMPLEMENT",
		"IMGUI_API=__declspec(dllexport)",
		"PH_ENGINE_EXPORT",
		"PH_STATIC_BUILD",
		"YAML_CPP_STATIC_DEFINE"
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

    