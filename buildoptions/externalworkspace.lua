outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "../dep/Dependencies.lua"



group "LabCore/core"
    include "../Base"
    include "../Platform"
    include "../Sandbox"
    include "../Engine"
    include "../LabCore"
group ""

group "LabCore/labcore-plugins"
    include "../LabCore/plugins/labcore-3d"
    include "../LabCore/plugins/labcore-plt"
    include "../LabCore/plugins/labcore-scene"
    include "../LabCore/plugins/labcore-ssh"
group ""

group "LabCore/projects"
    include "../projects/RP-Gui"
    include "../projects/RpfilterStudio"
group ""

group "LabCore/dependencies"
    include "../dep/imgui"
    --include "dep/libssh"
    --include "dep/stb"
    --include "dep/entt"
    include "../dep/yaml"
    include "../dep/assimp"
    --include "dep/ImGuizmo"
group ""


