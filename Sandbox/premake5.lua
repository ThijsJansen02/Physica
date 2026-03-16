
project "Sandbox"
	kind "SharedLib"
	language "C++"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/int")
	staticruntime "off"

	ignoredefaultlibraries { "libraries" }

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
		"%{IncludeDir.dep}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.engine}" 
	}

	defines 
	{
		"PH_SHARED_BUILD"
	}

	links
	{
		"Imgui",
		"base",
		"Engine"
	}

	postbuildcommands 
	{
		"{COPYFILE} %{cfg.buildtarget.abspath} %{wks.location}/bin/" .. outputdir .. "/platform",

	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"PH_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "PH_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PH_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PH_DIST"
		runtime "Release"
		optimize "on"

    