outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "../dep/Dependencies.lua"

group "core"
    include "../Base"
    include "../Platform"
    include "../Sandbox"
    include "../Engine"
    include "../LabCore"
group ""

group "labcore-plugins"
    include "../LabCore/plugins/labcore-3d"
    include "../LabCore/plugins/labcore-plt"
    include "../LabCore/plugins/labcore-scene"
    include "../LabCore/plugins/labcore-ssh"
group ""

group "projects"
    include "../projects/RP-Gui"
    include "../projects/RpfilterStudio"
group ""

group "dependencies"
    include "../dep/imgui"
    --include "dep/libssh"
    --include "dep/stb"
    --include "dep/entt"
    include "../dep/yaml"
    include "../dep/assimp"
    --include "dep/ImGuizmo"
group ""


