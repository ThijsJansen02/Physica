
project "Base"
    kind "StaticLib"
    language "C++"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/int")

	ignoredefaultlibraries { "libraries" }

    files 
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs
	{
        "%{IncludeDir.base}"
	}
    
    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
			"PH_PLATFORM_WINDOWS",
            "PH_BASE",
            "PH_STATIC_BUILD"
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