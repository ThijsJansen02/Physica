
#include <Editor/EditorAPI.h>

namespace PH::Editor {

    using namespace PH::Engine;
    SCRIPT_API Engine::Assets::ScriptInfo getInfo() {
        Assets::ScriptInfo info{};
        info.type = ScriptType::VIEW_PLUGIN;
        return info;
    }
}

