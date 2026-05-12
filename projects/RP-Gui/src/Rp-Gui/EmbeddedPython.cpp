
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#include <Engine/Engine.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/embed.h>

#include "RpGui.h"

namespace py = pybind11;

namespace PH::RpGui {

	static wchar_t* charToWChar(const char* text)
	{
		size_t size = strlen(text) + 1;
		wchar_t* wa = (wchar_t*)Engine::Allocator::alloc(size * sizeof(wchar_t));
		mbstowcs(wa, text, size);
		return wa;
	}
	
	void initPython(const char* pythonhome) {
		PyConfig config{};
		// 1. Initialize with default Python configuration
		PyConfig_InitIsolatedConfig(&config);

		config.isolated = 1;

		// This is important:
		config.install_signal_handlers = 1;

		// Ensure stdio is initialized properly
		config.buffered_stdio = 1;

		//set home
		const wchar_t* pyhome = charToWChar(pythonhome);
		PyConfig_SetString(&config, &config.home, pyhome);

		//PyConfig_SetString(&config, &config.path, pyhome);

		PyConfig_SetString(&config, &config.program_name, L"RP-GUI");

		config.module_search_paths_set = 1;

		INFO << "Python home set to: " << pythonhome << "\n";

		Engine::String libpath = Engine::String::create(pythonhome).append("\\Lib");
		const wchar_t* pylibpath = charToWChar(libpath.getC_Str());
		PyWideStringList_Append(&config.module_search_paths, pylibpath);

		PyWideStringList_Append(&config.module_search_paths, pyhome);

		Engine::String buildpath = Engine::String::create(pythonhome).append("\\python313.zip");
		const wchar_t* pybuildpath = charToWChar(buildpath.getC_Str());
		PyWideStringList_Append(&config.module_search_paths, pybuildpath);

		Engine::String packages = Engine::String::create(pythonhome).append("\\Lib\\site-packages");
		const wchar_t* pypackagespath = charToWChar(packages.getC_Str());
		PyWideStringList_Append(&config.module_search_paths, pypackagespath);

		try {
			py::initialize_interpreter(&config);
		}
		catch (py::error_already_set& e) {
			// This will print the actual Python error message and traceback to C++ stderr
			ERR << "Python Error: " << e.what() << "\n";
		}
		catch (std::runtime_error& e) {
			ERR << "Runtime Error: " << e.what() << "\n";
		}
		catch (std::exception& e) {
			ERR << "Exception: " << e.what() << "\n";
		}

		Engine::String::destroy(&libpath);
		Engine::String::destroy(&buildpath);
		Engine::String::destroy(&packages);

	}
}