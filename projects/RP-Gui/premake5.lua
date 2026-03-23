project "RP-GUI"
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
		--"%{IncludeDir.imguizmo}",
		"%{IncludeDir.editor}",

	}

	defines 
	{
		"PH_SCENE_IMPLEMENT",
		"IMGUI_API=__declspec(dllexport)",
		"PH_SHARED_BUILD",
		"PH_ENGINE_EXPORT",
		"YAML_CPP_STATIC_DEFINE"
	}

	links
	{
		"base",
		"Engine"
	}

	postbuildcommands 
	{
		"{COPYFILE} %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/%{cfg.buildtarget.name} %{wks.location}/bin/" .. outputdir .. "/Platform",
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

	filter "configurations:Release"
		defines "PH_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PH_DIST"
		runtime "Release"
		optimize "on"

    