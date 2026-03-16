#include "Script.h"
#include <Base/Datastructures/Stream.h>
namespace PH::Engine::Assets {

	//runs when the asset is included into the library
	bool32 Script::include(YAML::Node& root, Library* lib, Script* instance) {
		instance->scripttype = (ScriptType)root["ScriptType"].as<int32>();
		return true;
	}

	//serializes the asset into the yaml emitter
	void Script::serialize(YAML::Emitter& out, Script* instance) {
		
		
		out << YAML::BeginMap;
		out << YAML::Key << "SourcePath" << YAML::Value << instance->sourcepath.getC_str();
		out << YAML::Key << "BinPath" << YAML::Value << instance->binpath.getC_str();
		out << YAML::Key << "Language" << YAML::Value << (int32)instance->sourcelanguage;
		out << YAML::Key << "ScriptType" << YAML::Value << (int32)instance->scripttype;
		out << YAML::EndMap;
	}

	ScriptInfo loadScript(Script* script);
	bool32 compileCPPscript(Script* script);

	//deserializes the asset, the path parameter is only there for debug messages...
	bool32 Script::deserialize(YAML::Node& root, Library* lib, Script* result, const char* path) {
		
		result->sourcepath = root["SourcePath"].as<FilePath>();
		result->binpath = root["BinPath"].as<FilePath>();
		result->sourcelanguage = (ScriptSourceLanguage)root["Language"].as<int32>();
		result->scripttype = (ScriptType)root["ScriptType"].as<int32>();

		ScriptInfo info = loadScript(result);

		if (info.type == result->scripttype) {
			return true;
		}

		//if it is not possible to load the script immediatly, try to compile it from source..
		if (!compileCPPscript(result)) {
			return false;
		}

		info = loadScript(result);
		if (info.type == result->scripttype) {
			return true;
		}

		return false;
	}

	

	bool32 compileCPPscript(Script* script) {

		Engine::ArenaScope s;

		char currentdir[MAX_PATH] = {0};
		GetCurrentDirectoryA(MAX_PATH, currentdir);

		if (!script->sourcepath.isAbsolute()) {
			script->sourcepath.makeAbsolute(currentdir);
		}

		if (!script->binpath.isAbsolute()) {
			script->binpath.makeAbsolute(currentdir);
		}
		
		auto  commands = Base::Stream<Engine::ArenaAllocator>::create(1024);
		

		Base::String<ArenaAllocator> bindir = script->binpath.getDirectory<ArenaAllocator>();
		Base::String<ArenaAllocator> filename = script->binpath.getFileName<ArenaAllocator>();

		script->binpath.useBackSlashes();
		script->sourcepath.useBackSlashes();

		//commands << "cd " << script->binpath.getC_str() << "&& ";

		commands << "call \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall\" x64 ";

		commands << "&& cd " << bindir.getC_Str() << " && ";

		//compiler flags
		commands << "cl " << " /std:c++17 /Zi /MDd /EHsc /Fe:" << filename.getC_Str() <<  ".dll ";
		
		//preprocessor
		commands << "/DIMGUI_API=__declspec(dllimport) /DPH_SHARED_BUILD /DYAML_CPP_STATIC_DEFINE ";

		//should be done with a relative path in the future
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\Editor\\src ";
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\Engine\\src ";
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\Platform\\src ";
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\Base\\src ";
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\dep\\imgui ";
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\dep\\yaml-cpp\\include ";
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\dep\\entt\\src ";
		commands << "/I" << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\dep\\glm\\include ";
		
		//src files
		commands << script->sourcepath.getC_str() << " ";

		//libs
		commands << "C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\ProjectPhysica\\bin\\Debug-windows-x86_64\\Editor\\Editor.lib ";

		//linker settings 
		commands << "/link /MAP /DLL /DEBUG:NONE";
		system((char*)commands.raw());

		script->binpath.validate();
		script->sourcepath.validate();

		return true;
	}

	typedef ScriptInfo getInfoFN();

	ScriptInfo loadScript(Script* script) {

		script->loadedbinaries = LoadLibraryA(script->binpath.getC_str());
		if (script->loadedbinaries == 0) {
			return {};
		}

		getInfoFN* getinfo = (getInfoFN*)GetProcAddress(script->loadedbinaries, "getInfo");

		if (getinfo) {
			ScriptInfo info = getinfo();
			return info;
		}

		return ScriptInfo{};

	}

	void unloadScript(Script* script) {
		FreeLibrary(script->loadedbinaries);
		script->loadedbinaries = NULL;
	}

	bool32 importScriptFromCppFile(const char* sourcepath, const char* binpath, Script* script) {


		script->sourcepath = Base::FilePath<Engine::Allocator>::create(sourcepath);
		script->binpath = Base::FilePath<Engine::Allocator>::create(binpath);
		Engine::String filename = script->sourcepath.getFileName();

		script->binpath.m_Str.append("/").append(filename.getC_Str()).append(".dll");


		if (!compileCPPscript(script)) {
			return false;
		}

		ScriptInfo scriptinfo =  loadScript(script);
		if (scriptinfo.type == ScriptType::UNDEFINED) {
			return false;
		}

		script->scripttype = scriptinfo.type;

		return true;
	}
}