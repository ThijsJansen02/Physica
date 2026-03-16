project "Platform"
	kind "WindowedApp"
	language "C++"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/int")
	debugdir "%{wks.location}/Editor"

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
		"%{IncludeDir.vulkanSDK}"
	}

	libdirs
	{
		"dep/lib",
		"%{LibraryDir.vulkanSDK}"
	}

	links 
	{
		"Base",
		"Imgui",
		"%{Library.vulkan}"
	}

	dependson {
		"Editor"
	}

	defines 
	{
		"PH_STATIC_BUILD"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"PH_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "PH_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.shaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter "configurations:Release"
		defines "PH_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.shaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Dist"
		defines "PH_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.shaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

    