
#include "Engine/Asset.h"
#include <Windows.h>

namespace PH::Engine::Assets {

#define SCRIPT_API extern "C" __declspec(dllexport)

	enum struct ScriptSourceLanguage {
		C_CPP
	};

	enum struct ScriptType {
		UNDEFINED = 0,
		VIEW_PLUGIN,
		ENTITY_SCRIPT,
		RUNABLE
	};

	struct ScriptInfo {
		ScriptType type;
	};

	struct Script : public Asset {

		Base::FilePath<Engine::Allocator> sourcepath;
		Base::FilePath<Engine::Allocator> binpath;
		ScriptSourceLanguage sourcelanguage;
		ScriptType scripttype;

		static constexpr const char* getExtension() {
			return ".phscript";
		}

		static constexpr AssetType getType() {
			return AssetType::SCRIPT;
		}
		
		//TODO: needs to be moved to the platform side
		HMODULE loadedbinaries;
		

		friend class Library;
	private:

		//runs when the asset is included into the library
		static bool32 include(YAML::Node& root, Library* lib, Script* instance);
		//serializes the asset into the yaml emitter
		static void serialize(YAML::Emitter& out, Script* instance);
		//deserializes the asset, the path parameter is only there for debug messages...
		static bool32 deserialize(YAML::Node& root, Library* lib, Script* result, const char* path);

	};

	void unloadScript(Script* script);
	ScriptInfo loadScript(Script* script);
	bool32 importScriptFromCppFile(const char* sourcepath, const char* binpath, Script* script);
	bool32 compileCPPscript(Script* script);


}