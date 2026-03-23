include "Dependencies.lua"

workspace "ProjectPhysica"
    architecture "x64"
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }
    startproject "Platform"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


group "core"
    include "Base"
    include "Platform"
    include "Sandbox"
    include "Engine"
    include "Editor"
group ""

group "projects"
    include "projects/RP-Gui"
group ""

group "dependencies"
    include "dep/imgui"
    --include "dep/stb"
    --include "dep/entt"
    include "dep/yaml"
    include "dep/assimp"
    --include "dep/ImGuizmo"
group ""


