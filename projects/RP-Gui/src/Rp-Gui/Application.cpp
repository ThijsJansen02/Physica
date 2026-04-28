#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

//must be included before windows.h to avoid issues with the winsock2.h header that is included by libssh, because windows.h defines some macros that can cause issues with the winsock2.h header if it is included after windows.h
#include "rpconnection.h"


#include "RpGui.h"

#include <Platform/PlatformAPI.h>
#include <Base/Log.h>

#include <Engine/cppAPI/Rendering.hpp>
#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/imgui/DockSpace.h>
#include <Engine/YamlExtensions.h>

#include "TransferFunction.h"

#include "Context.h"
#include "imgui_spectrum.h"

#define PY_SSIZE_T_CLEA


#ifdef _DEBUG
	#undef _DEBUG
	#include <Python.h>
	#define _DEBUG
#endif

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/embed.h>


#include "Text.h"
#include "Plot.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

using namespace PH;

namespace PH::RpGui {
	Context* context = nullptr;
	Engine::Renderer2D::Wrapper renderer2D;

	PH::Base::LogStream<consoleWrite> INFO;
	PH::Base::LogStream<consoleWrite> WARN;
	PH::Base::LogStream<consoleWrite> ERR;
}

using namespace PH::RpGui;


RpGui::TransferFunction createExampleTransferFunction() {
	TransferFunction examplefunction{};

	examplefunction.currentcommand = Engine::String::create("");
	examplefunction.name = Engine::String::create("example function");
	examplefunction.connection.remoteip = Engine::String::create("root@rp-f083c2.local");

	examplefunction.filters = Engine::ArrayList<Filter>::create(3);
	Filter filter1{};
	filter1.cutoff = 1000.0f;
	filter1.gain = 1.0f;
	filter1.Qfactor = 30.0f;
	filter1.type = FilterType::BANDPASS;

	examplefunction.filters.pushBack(filter1);
	filter1.cutoff = 4000.0f;
	examplefunction.filters.pushBack(filter1);
	filter1.cutoff = 10000.0f;
	examplefunction.filters.pushBack(filter1);

	return examplefunction;
}

namespace py = pybind11;

#define M_PI 3.14159265359

real64 targetfs = 125e6 / 256.0;

void sentFilterToRp(Filter f, real64 targetfs, RpGui::RpConnection* connection) {

	//if the filter type is resonance anti resonance the cutoff is average between both resonances, so we need to calculate the cutoff differently
	f.cutoff = prewarp(2 * M_PI * f.cutoff, targetfs);

	BiQuadCoefficients dcoeffs = bilinearTransform(calculateCoefficients(f), targetfs);

	Engine::String rpcommand = generateRPfilterString(dcoeffs);
	if (connection->open) {
		connection->commandqueue.push({ rpcommand });
		ReleaseSemaphore(connection->semaphore, 1, nullptr);
	}
}

//actually usable functions in python!
void setFilterCutoff(int filternumber, float cutoff) {
	RpGui::context->activetransferfunctions[0].filters[filternumber].cutoff = cutoff;

	recalculateFilter(&RpGui::context->activetransferfunctions[0].filters[filternumber]);	
	sentFilterToRp(RpGui::context->activetransferfunctions[0].filters[filternumber], targetfs, &RpGui::context->activetransferfunctions[0].connection);
}

//actually usable functions in python!
void setFilterQfactor(int filternumber, float qfactor) {
	RpGui::context->activetransferfunctions[0].filters[filternumber].Qfactor = qfactor;

	recalculateFilter(&RpGui::context->activetransferfunctions[0].filters[filternumber]);
	sentFilterToRp(RpGui::context->activetransferfunctions[0].filters[filternumber], targetfs, &RpGui::context->activetransferfunctions[0].connection);
}

//actually usable functions in python!
void addPlot(py::array_t<float> freq, py::array_t<float> magnitude, char* name) {
	
	auto freqbuf = freq.request();
	float* freqptr = static_cast<float*>(freqbuf.ptr);
	size_t freqsize = freqbuf.size;

	auto magbuf = magnitude.request();
	float* magptr = static_cast<float*>(magbuf.ptr);
	size_t magsize = magbuf.size;

	Engine::ArrayList<glm::vec2> points = Engine::ArrayList<glm::vec2>::create(freqsize);
	for (uint32 i = 0; i < freqsize; i++) {
		points.pushBack({ freqptr[i], magptr[i] });
	}

	INFO << "Adding plot with name: " << name << " and " << freqsize << " points\n";

	RpGui::PlotData plotdata{};
	plotdata.name = Engine::String::create(name);
	plotdata.data = points;

	RpGui::context->openedplots.pushBack(plotdata);
}

//removes a plot
void removePlot(char* name) {
	sizeptr index = 0;
	for (auto& plt : RpGui::context->openedplots) {
		if (Base::stringCompare(name, plt.name.getC_Str()) == true) {

			//release the memory of the plot data and name
			Engine::ArrayList<glm::vec2>::destroy(&plt.data);
			Engine::String::destroy(&plt.name);	

			INFO << "Removing plot with name: " << name << "\n";
			RpGui::context->openedplots.remove(index);
			index++;
			return;
		}
	}
}

void setFont(char* font, int bitmapsize, int charactersize) {

	RpGui::Font newfont = loadFont(font, bitmapsize, charactersize);
	if (newfont.bitmapwidth == 0) {
		ERR << "Failed to load font\n";
		return;
	}

	/*
	PH::Platform::GFX::destroyTextures(&RpGui::context->font.atlas, 1);	
	PH::Platform::GFX::destroyBuffers(&RpGui::context->font.cdatabuffer, 1);
	Engine::Allocator::dealloc(RpGui::context->font.cdata_cpu);
	PH::Platform::GFX::destroyDescriptorSets(&RpGui::context->font.cdata, 1);
	*/
	RpGui::context->font = newfont;
}

PYBIND11_EMBEDDED_MODULE(hostimgui, m) {
	m.doc() = "exposed imgui function for use in python";

	py::class_<ImVec2>(m, "Vec2")
		.def(py::init<float, float>())
		.def_readwrite("x", &ImVec2::x)
		.def_readwrite("y", &ImVec2::y);

	py::class_<ImVec4>(m, "Vec4")
		.def(py::init<float, float, float, float>())
		.def_readwrite("x", &ImVec4::x)
		.def_readwrite("y", &ImVec4::y)
		.def_readwrite("z", &ImVec4::z)
		.def_readwrite("w", &ImVec4::w);

	py::enum_<ImGuiCol_>(m, "Col")
		.value("Text", ImGuiCol_Text)
		.value("TextDisabled", ImGuiCol_TextDisabled)
		.value("WindowBg", ImGuiCol_WindowBg)
		.value("ChildBg", ImGuiCol_ChildBg)
		.value("PopupBg", ImGuiCol_PopupBg)
		.value("Border", ImGuiCol_Border)
		.value("FrameBg", ImGuiCol_FrameBg)
		.value("FrameBgHovered", ImGuiCol_FrameBgHovered)
		.value("FrameBgActive", ImGuiCol_FrameBgActive)
		.value("TitleBg", ImGuiCol_TitleBg)
		.value("TitleBgActive", ImGuiCol_TitleBgActive)
		.value("Button", ImGuiCol_Button)
		.value("ButtonHovered", ImGuiCol_ButtonHovered)
		.value("ButtonActive", ImGuiCol_ButtonActive)
		.value("Header", ImGuiCol_Header)
		.value("HeaderHovered", ImGuiCol_HeaderHovered)
		.value("HeaderActive", ImGuiCol_HeaderActive)
		.value("Tab", ImGuiCol_Tab)
		.value("TabActive", ImGuiCol_TabActive)
		.value("TabHovered", ImGuiCol_TabHovered)
		.value("TabSelected", ImGuiCol_TabSelected)
		.value("TabUnfocussed", ImGuiCol_TabUnfocused)
		.value("TabDimmed", ImGuiCol_TabDimmed)
		.value("TabDimmedSelectedOverline", ImGuiCol_TabDimmedSelectedOverline)
		.value("TabSelectedOverline", ImGuiCol_TabSelectedOverline)
			.export_values();

	py::class_<ImGuiStyle>(m, "Style")
		// --- main scalar values ---
		.def_readwrite("alpha", &ImGuiStyle::Alpha)
		.def_readwrite("disabled_alpha", &ImGuiStyle::DisabledAlpha)
		.def_readwrite("window_rounding", &ImGuiStyle::WindowRounding)
		.def_readwrite("window_border_size", &ImGuiStyle::WindowBorderSize)
		.def_readwrite("child_border_size", &ImGuiStyle::ChildBorderSize)
		.def_readwrite("popup_border_size", &ImGuiStyle::PopupBorderSize)
		.def_readwrite("frame_rounding", &ImGuiStyle::FrameRounding)
		.def_readwrite("frame_border_size", &ImGuiStyle::FrameBorderSize)
		.def_readwrite("indent_spacing", &ImGuiStyle::IndentSpacing)
		.def_readwrite("columns_min_spacing", &ImGuiStyle::ColumnsMinSpacing)
		.def_readwrite("scrollbar_size", &ImGuiStyle::ScrollbarSize)
		.def_readwrite("scrollbar_rounding", &ImGuiStyle::ScrollbarRounding)
		.def_readwrite("grab_min_size", &ImGuiStyle::GrabMinSize)
		.def_readwrite("grab_rounding", &ImGuiStyle::GrabRounding)
		.def_readwrite("log_slider_deadzone", &ImGuiStyle::LogSliderDeadzone)
		.def_readwrite("tab_rounding", &ImGuiStyle::TabRounding)
		.def_readwrite("tab_border_size", &ImGuiStyle::TabBorderSize)
		.def_readwrite("mouse_cursor_scale", &ImGuiStyle::MouseCursorScale)

		// --- alignment / enums (stored as ints) ---
		.def_readwrite("window_menu_button_position",
			&ImGuiStyle::WindowMenuButtonPosition)
		.def_readwrite("color_button_position",
			&ImGuiStyle::ColorButtonPosition)

		// --- booleans ---
		.def_readwrite("anti_aliased_lines", &ImGuiStyle::AntiAliasedLines)
		.def_readwrite("anti_aliased_lines_use_tex",
			&ImGuiStyle::AntiAliasedLinesUseTex)
		.def_readwrite("anti_aliased_fill", &ImGuiStyle::AntiAliasedFill)

		// --- vec2 fields ---
		.def_readwrite("window_padding", &ImGuiStyle::WindowPadding)
		.def_readwrite("window_min_size", &ImGuiStyle::WindowMinSize)
		.def_readwrite("window_title_align", &ImGuiStyle::WindowTitleAlign)
		.def_readwrite("frame_padding", &ImGuiStyle::FramePadding)
		.def_readwrite("item_spacing", &ImGuiStyle::ItemSpacing)
		.def_readwrite("item_inner_spacing", &ImGuiStyle::ItemInnerSpacing)
		.def_readwrite("cell_padding", &ImGuiStyle::CellPadding)
		.def_readwrite("touch_extra_padding", &ImGuiStyle::TouchExtraPadding)
		.def_readwrite("display_window_padding", &ImGuiStyle::DisplayWindowPadding)
		.def_readwrite("display_safe_area_padding", &ImGuiStyle::DisplaySafeAreaPadding)
		.def_readwrite("selectable_text_align", &ImGuiStyle::SelectableTextAlign)

		// --- curve tuning ---
		.def_readwrite("curve_tessellation_tol",
			&ImGuiStyle::CurveTessellationTol)
		.def_readwrite("circle_tessellation_max_error",
			&ImGuiStyle::CircleTessellationMaxError);

	m.def("getStyle", []() -> ImGuiStyle& {
		return ImGui::GetStyle();
		}, py::return_value_policy::reference);

	m.def("getColor", [](ImGuiCol_ col) -> ImVec4& {
		return ImGui::GetStyle().Colors[col];
		}, py::return_value_policy::reference);

	m.def("setColor", [](ImGuiCol_ col, ImVec4 c) {
		ImGui::GetStyle().Colors[col] = c;
		});
}

// Bind it to a Python module
PYBIND11_EMBEDDED_MODULE(RpGui, m) {
	m.doc() = "C++ functions for my RpGui";
	m.def("setFilterCutoff", &setFilterCutoff, "sets the cutoff of a filter",
		py::arg("a"), py::arg("b"));
	m.def("setFilterQfactor", &setFilterQfactor, "set the qfactor of a filter",
		py::arg("filternumber"), py::arg("qfactor"));
	m.def("addPlot", &addPlot, "adds a plot to the GUI with the given frequency and magnitude data and name",
		py::arg("freq"), py::arg("magnitude"), py::arg("name"));
	m.def("removePlot", &removePlot, "removes a plot from the GUI with the given name",
		py::arg("name"));
}

static wchar_t* charToWChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = (wchar_t*)Engine::Allocator::alloc(size * sizeof(wchar_t));
	mbstowcs(wa, text, size);
	return wa;
}

void initPython() {
	PyConfig config{};
	// 1. Initialize with default Python configuration
	PyConfig_InitIsolatedConfig(&config);

	config.isolated = 1;

	// This is important:
	config.install_signal_handlers = 1;

	// Ensure stdio is initialized properly
	config.buffered_stdio = 1;

	//set home
	const wchar_t* pyhome = charToWChar(RpGui::context->pythonhome.getC_Str());
	PyConfig_SetString(&config, &config.home, pyhome);

	//PyConfig_SetString(&config, &config.path, pyhome);

	PyConfig_SetString(&config, &config.program_name, L"RP-GUI");	

	config.module_search_paths_set = 1;

	INFO << "Python home set to: " << RpGui::context->pythonhome.getC_Str() << "\n";

	Engine::String libpath = Engine::String::create(RpGui::context->pythonhome.getC_Str()).append("\\Lib");
	const wchar_t* pylibpath = charToWChar(libpath.getC_Str());
	PyWideStringList_Append(&config.module_search_paths, pylibpath);

	PyWideStringList_Append(&config.module_search_paths, pyhome);

	Engine::String buildpath = Engine::String::create(RpGui::context->pythonhome.getC_Str()).append("\\python313.zip");
	const wchar_t* pybuildpath = charToWChar(buildpath.getC_Str());
	PyWideStringList_Append(&config.module_search_paths, pybuildpath);

	Engine::String packages = Engine::String::create(RpGui::context->pythonhome.getC_Str()).append("\\Lib\\site-packages");
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
}

Engine::DynamicArray<uint8> checkCompileBinaries(const char* path, Platform::GFX::ShaderStageFlags stage) {

	Platform::FileBuffer buffer;
	Engine::String binpath = Engine::String::create(path).append(".bin");

	Engine::DynamicArray<uint8> result;

	if (Platform::loadFile(&buffer, binpath.getC_Str())) {
		result = Engine::DynamicArray<uint8>::create(buffer.size);
		Base::copyMemory(buffer.data, result.raw(), buffer.size);
		Platform::unloadFile(&buffer);
	}
	
	//no binaries exist, try to compile the shader source and save the binaries for later use, so we don't have to compile the shader every time we run the application, which can be slow, especially on older hardware. This is a simple caching mechanism that can significantly improve load times after the first run.
	else if (Platform::loadFile(&buffer, path)) {
		result = Engine::Renderer2D::compileGLSLSourceToVulkanBinary((const char*)buffer.data, stage);

		Platform::FileBuffer writebuffer{};
		writebuffer.data = result.raw();
		writebuffer.size = result.getCapacity();

		INFO << "Compiled shader source from path: " << path << " to binary and saved it to path: " << binpath.getC_Str() << "\n";

		Platform::writeFile(writebuffer, binpath.getC_Str());
		Platform::unloadFile(&buffer);
	}
	else {
		ERR << "Failed to load shader source from path: " << path << "\n";
	}

	Engine::String::destroy(&binpath);
	return result;
}

PH_DLL_EXPORT PH_APPLICATION_INITIALIZE(applicationInitialize) {

	//sets up the engine allocators and other systems that rely on the engine allocator, such as the console log stream
	PH::Engine::EngineInitInfo engineinit{};
	engineinit.memory = (PH::uint8*)context.appmemory;
	engineinit.memorysize = context.appmemsize;
	engineinit.platformcontext = &context;
	PH::Engine::init(engineinit);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_FrameBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 1.0f };
	

	ssh_init(); //libssh test


	RpGui::context = (RpGui::Context*)Engine::Allocator::alloc(sizeof(RpGui::Context));

	RpGui::context->magnitudeplot = RpGui::PlotViewPanel::create({ -10.0f, -10.0f, 10.0f, 10.0f }, "magnitude");
	RpGui::context->phaseplot = RpGui::PlotViewPanel::create({ -10.0f, -180.0f, 10.0f, 180.0f }, "phase");

	RpGui::context->openedplots = Engine::ArrayList<PlotData>::create(1);

	//lock the xaxis for both plots together
	RpGui::context->magnitudeplot.xlock = &RpGui::context->phaseplot;
	RpGui::context->phaseplot.xlock = &RpGui::context->magnitudeplot;

	RpGui::context->openproject = Engine::String::create("project1.rpproj");


	auto ini = Engine::FileIO::loadYamlfile("RpGui.ini");
	//loading the application settings
	if (ini) {

		const auto& plotviewpanels = ini["PlotViewPanels"];
		if (plotviewpanels) {
			RpGui::context->magnitudeplot.deserialize(plotviewpanels["magnitude"]);
			RpGui::context->phaseplot.deserialize(plotviewpanels["phase"]);
		}


		auto currentprojectdir = ini["CurrentProject"];
		if (currentprojectdir) {
			//should fix this at some point, Engine::string never gets deleted!!
			RpGui::context->openproject.set(currentprojectdir.as<Engine::String>());
		}

		auto pythonhome = ini["PythonHome"];
		if (pythonhome) {
			RpGui::context->pythonhome = pythonhome.as<Engine::String>();
		}
		else {
			char buffer[256];
			GetCurrentDirectoryA(256, buffer);
			RpGui::context->pythonhome = Engine::String::create(buffer);
			RpGui::context->pythonhome.append("\\..\\..\\dep\\embeddedpython");
		}
	}

#if 1
	initPython();
#endif

	RpGui::context->font = RpGui::loadFont("c:/windows/fonts/arial.ttf", 512, 32.0f);

	auto defaultquadvert = checkCompileBinaries("res/shaders/default_quadshader.vert", Platform::GFX::SHADER_STAGE_VERTEX_BIT);
	auto defaultquadfrag = checkCompileBinaries("res/shaders/default_quadshader.frag", Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

	RpGui::context->pipeline2D = Engine::Renderer2D::createGraphicsPipelineFromBinaries(&RpGui::context->magnitudeplot.display,
		defaultquadvert.getArray(),
		defaultquadfrag.getArray(),
		{ nullptr, 0 }
	);

	Engine::DynamicArray<uint8>::destroy(&defaultquadvert);
	Engine::DynamicArray<uint8>::destroy(&defaultquadfrag);

	auto defaultfontvert = checkCompileBinaries("res/shaders/default_fontshader.vert", Platform::GFX::SHADER_STAGE_VERTEX_BIT);
	auto defaultfontfrag = checkCompileBinaries("res/shaders/default_fontshader.frag", Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

	RpGui::context->fontpipeline2D = Engine::Renderer2D::createGraphicsPipelineFromBinaries(&RpGui::context->magnitudeplot.display,
		defaultfontvert.getArray(),
		defaultfontfrag.getArray(),
		{ &RpGui::fontuserlayout, 1 }
	);

	Engine::DynamicArray<uint8>::destroy(&defaultfontvert);
	Engine::DynamicArray<uint8>::destroy(&defaultfontfrag);
	


	//setup example transferfunctions; should in the future be loaded from a serialized document
	RpGui::context->activetransferfunctions = Engine::ArrayList<TransferFunction>::create(1);

	auto proj = Engine::FileIO::loadYamlfile(RpGui::context->openproject.getC_Str());
	if (proj) {
		auto transferfunctions = proj["TransferFunctions"];
		for (const auto& tf : transferfunctions) {
			RpGui::context->activetransferfunctions.pushBack(deserializeTransferFunction(tf));
		}
	}
	else {
		RpGui::context->activetransferfunctions.pushBack(createExampleTransferFunction());
	}

	//open connections with for all transferfunction with a remote
	for (auto& tf : RpGui::context->activetransferfunctions) {

		if (!tf.connection.remoteip.isEmpty()) {

			tf.connection.semaphore = CreateSemaphoreEx(0, 0, 1, nullptr, 0, SEMAPHORE_ALL_ACCESS);
			tf.connection.commandqueue = PH::Base::CircularWorkQueue<RpCommand, Engine::Allocator>::create(30);

			PH::Platform::ThreadCreateInfo threadinfo{};
			threadinfo.threadworkmemorysize = 0;
			threadinfo.usegfx = false;
			threadinfo.userdata = (void*)&tf.connection;
			threadinfo.threadproc = rp_connection_thread;

			PH::Platform::createThread(threadinfo, &tf.connection.thread);

			ReleaseSemaphore(tf.connection.semaphore, 1, nullptr);
			tf.connection.commandqueue.push({ Engine::String::create("export PATH=$PATH:/opt/redpitaya/bin;fpgautil - b sinewave_generator_wrapper.bit.bin") });

			for (auto f : tf.filters) {
				sentFilterToRp(f, targetfs, &tf.connection);
			}
		}
	}


	Engine::Renderer2D::InitInfo init{};
	init.currentpipeline = RpGui::context->pipeline2D;
	init.descriptorsetlayouts = { nullptr, 0 };
	init.instancebuffersize = 8 * MEGA_BYTE;
	init.shadowmapdimensions = 0;

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;


	RpGui::context->buffer = Engine::ArrayList<glm::vec2>::create(10);

	RpGui::renderer2D = Engine::Renderer2D::Wrapper::create(init);
	return true;
}
	//this is going to be the function that draws the plot, it takes in the vertices of the plot, the range of the plot and the region of the plot, and it draws the plot using the renderer2D wrapper, this is going to be called from the drawTransferFunctionMagnitude and drawTransferFunctionPhase functions, which are going to generate the vertices for the plot based on the transfer function and then call this function to draw the plot, this is going to allow us to separate the logic of generating the vertices for the plot from the logic of drawing the plot, which is going to make it easier to maintain and extend in the future, for example if we want to add support for different types of plots or different types of data sources for the plots, we can just generate different vertices for those plots and then call this function to draw them without having to duplicate any code.

void drawTransferFunctionMagnitude(PlotViewPanel* plot, TransferFunction* function, Engine::ArrayList<glm::vec2>* buffer) {

	static int32 nsamples = 2000;

	real64 xrange = plot->range.right - plot->range.left;
	real64 dx = xrange / (real64)nsamples;

	buffer->clear();

	//draw the specified function, is going to change in the future to allow for different functions and parameters, for now its just a bandpass filter

	//this is dangerous because if there is an floating point error in dx than this function can blow up!
	for (real64 x = plot->range.left; x <= plot->range.right; x += dx) {

		Base::Complex<real64> y = 1.0f;

		for (auto& filter : function->filters) {
			y = y * applyFilter(pow(10.0f, x) * Base::Complex<real64>::i(), filter.coeffs);
		}


		buffer->pushBack(glm::vec2{x, 20.0f * log10f(y.modulus())});
	}

	drawPlot(buffer->getArray(), plot->range, plot->region);
}

void drawTransferFunctionPhase(PlotViewPanel* plot, TransferFunction* function, Engine::ArrayList<glm::vec2>* buffer) {

	static int32 nsamples = 2000;

	real32 xrange = plot->range.right - plot->range.left;
	real32 dx = xrange / (real32)nsamples;

	buffer->clear();

	//draw the specified function, is going to change in the future to allow for different functions and parameters, for now its just a bandpass filter
	for (real32 x = plot->range.left; x <= plot->range.right; x += dx) {

		Base::Complex<real64> y = 1.0f;

		for (auto& filter : function->filters) {
			y = y * applyFilter(pow(10.0f, x) * Base::Complex<real64>::i(), filter.coeffs);
		}


		buffer->pushBack(glm::vec2{ x, y.arg() });
	}

	drawPlot(buffer->getArray(), plot->range, plot->region);
}

typedef void (*UIfunc) (void*, RpGui::Context*);

template<UIfunc func>
inline void drawComponent(const Engine::String& name, PH::RpGui::Context* context, void* comp)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
	
	ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

	
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
	float lineHeight = 18.0f;
	bool open = ImGui::TreeNodeEx((void*)PH::Base::uint32Hash((uint32)name.getChar(0)), treeNodeFlags, name.getC_Str());
	//ImGui::PopStyleVar();


	ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
	if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
	{
		ImGui::OpenPopup("Settings");
	}

	bool removeComponent = false;
	if (ImGui::BeginPopup("Settings"))
	{
		if (ImGui::MenuItem("Remove component"))
			removeComponent = true;

		ImGui::EndPopup();
	}

	if (open)
	{
		func(comp, context);
		ImGui::TreePop();
	}

	if (removeComponent) {
			
	}
}

real32 dragspeed = 0.002;

void drawRpConnectionGui(void* function, RpGui::Context* context) {

	RpGui::TransferFunction* tf = (RpGui::TransferFunction*)function;



	char buffer[256];
	PH::Base::stringCopy(tf->connection.remoteip.getC_Str(), buffer, 256);


	uint32 id = 0;
	ImGui::PushID(id++);
	if (ImGui::InputText("", buffer, 256)) {
		tf->connection.remoteip.set(buffer);
	}
	ImGui::PopID();

	ImGui::SameLine();
	if (ImGui::Button("connect")) {
		if (tf->connection.open) {
			TerminateThread(tf->connection.thread.handle, 0);
		}

		//should become a standalone function at some point, so we can reuse it for multiple connections and not have to duplicate the code for each connection, also should be able to handle multiple connections at the same time, which is currently not possible because the connection information is stored in the transfer function, which is currently only one per application, but in the future we want to have multiple transfer functions with different connections, so we need to move the connection information to a separate struct that can be stored in the transfer function and passed to the connection thread, so we can have multiple connections at the same time and not have to duplicate the code for each connection.
		PH::Platform::ThreadCreateInfo threadinfo{};
		threadinfo.threadworkmemorysize = 0;
		threadinfo.usegfx = false;
		threadinfo.userdata = (void*)&tf->connection;
		threadinfo.threadproc = rp_connection_thread;

		PH::Platform::createThread(threadinfo, &tf->connection.thread);

		ReleaseSemaphore(tf->connection.semaphore, 1, nullptr);
		tf->connection.commandqueue.push({ Engine::String::create("export PATH=$PATH:/opt/redpitaya/bin;fpgautil - b sinewave_generator_wrapper.bit.bin") });
	}



	ImGui::PushID(id++);
	PH::Base::stringCopy(tf->currentcommand.getC_Str(), buffer, 256);
	if (ImGui::InputText("", buffer, 256)) {
		tf->currentcommand.set(buffer);
	}
	ImGui::PopID();

	ImGui::SameLine();
	if (ImGui::Button("send")) {
		if (tf->connection.connected) {

			PH::RpGui::RpCommand c{};
			tf->connection.commandqueue.push({ Engine::String::create(tf->currentcommand.getC_Str()) });
			ReleaseSemaphore(tf->connection.semaphore, 1, nullptr);
			tf->currentcommand.set("");
		}
		else {
			INFO << "red pitaya with adress " << tf->connection.remoteip.getC_Str() << "is not yet connected!\n";
		}
	}

	real64 targetfs = 125000000.0 / 256.0;

	for (auto& f : tf->filters) {
		ImGui::PushID(id);

		ImGui::Text("filter n%u", id);
		if (ImGui::BeginCombo("type", RpGui::FilterTypeStrings[f.type])) {

			for (uint32 filtertype = 0; filtertype < FILTER_TYPE_COUNT; filtertype++) {

				bool selected = (f.type == filtertype);

				if (ImGui::Selectable(RpGui::FilterTypeStrings[filtertype], selected)) {
					
					f.type = (FilterType)filtertype;
					recalculateFilter(&f);
					sentFilterToRp(f, targetfs, &tf->connection);
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (f.type == FilterType::RESONANCE_ANTI_RESONANCE) {

			if (ImGui::DragFloat("characteristic frequency", &f.cutoff, f.cutoff * dragspeed)) {
				recalculateFilter(&f);
				sentFilterToRp(f, targetfs, &tf->connection);

			}
			if (ImGui::DragFloat("Q factor", &f.Qfactor, f.Qfactor * dragspeed)) {
				recalculateFilter(&f);

				sentFilterToRp(f, targetfs, &tf->connection);

			}

			if (ImGui::DragFloat("Df", &f.df, f.df* dragspeed)) {
				recalculateFilter(&f);
				sentFilterToRp(f, targetfs, &tf->connection);
			}
			
			if (ImGui::DragFloat("anti resonant Q factor", &f.antiQfactor, f.antiQfactor * dragspeed)) {
				recalculateFilter(&f);
				sentFilterToRp(f, targetfs, &tf->connection);
			}

		}
		else {
			if (ImGui::DragFloat("Cutoff", &f.cutoff, f.cutoff * dragspeed)) {
				recalculateFilter(&f);
				sentFilterToRp(f, targetfs, &tf->connection);

			}
			if(ImGui::DragFloat("Q factor", &f.Qfactor, f.Qfactor * dragspeed)) {
				recalculateFilter(&f);
				sentFilterToRp(f, targetfs, &tf->connection);
			}
		}



		ImGui::PopID();
		id++;
		//ImGui::DragFloat("Cutoff", &f.cutoff, f.cutoff * dragspeed);
	}


}


PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame(&context);
	PH::Engine::Events::startNewFrame();

	

	real32 scrollspeed = 0.001f;

	//update all events events;
	for (auto& event : context.events) {
		PH::Engine::Events::onEvent(event);
		RpGui::context->magnitudeplot.onEvent(&event);
		RpGui::context->phaseplot.onEvent(&event);
	}

	auto& io = ImGui::GetIO();


	RpGui::renderer2D.begin();

	PH::Engine::BeginDockspace();

	ImGui::BeginMenuBar();
	ImGui::MenuItem("File");
	ImGui::EndMenuBar();

	static real32 textscale = 0.5f;


	//draw the magnitude plot for the bandpass filter
	RpGui::PlotViewPanel* plot = &RpGui::context->magnitudeplot;
	plot->beginRenderPass();
	
	//start drawing the plot, first set the pipeline and the view and projection matrices, then draw the background and the plot itself, then end the renderer and flush it to the GPU
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->pipeline2D);
	RpGui::renderer2D.pushView(glm::mat4(1.0f));
	RpGui::renderer2D.pushProjection(glm::ortho(0.0f, (real32)plot->region.right, (real32)plot->region.top, 0.0f));

	//draw the plot with lines and the plot itself
	drawPlotScaleLines(plot->range, plot->region);
	for (auto& transferfunction : RpGui::context->activetransferfunctions) {
		drawTransferFunctionMagnitude(plot, &transferfunction, &RpGui::context->buffer);
	}

	for (auto& plt : RpGui::context->openedplots) {

		Engine::ArrayList<glm::vec2> data = Engine::ArrayList<glm::vec2>::create(plt.data.getArray());

		drawPlot(data.getArray(), plot->range, plot->region, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f});

		Engine::ArrayList<glm::vec2>::destroy(&data);
	}

	//start drawing the text
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->fontpipeline2D, { &RpGui::context->font.cdata, 1 });
	RpGui::renderer2D.pushTexture(RpGui::context->font.atlas);
	RpGui::drawPlotScaleValues(plot->range, plot->region, &RpGui::context->font, textscale);

	RpGui::renderer2D.flush({ nullptr, 0 });

	//end renderpass for this display
	plot->endRenderPass();

	//draw the phase plot which is now just exactly the same as the magnitude plot
	plot = &RpGui::context->phaseplot;
	plot->beginRenderPass();

	//start drawing the plot, first set the pipeline and the view and projection matrices, then draw the background and the plot itself, then end the renderer and flush it to the GPU
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->pipeline2D);
	RpGui::renderer2D.pushView(glm::mat4(1.0f));
	
	//fix set projection to allow multiple windows!
	RpGui::renderer2D.pushProjection(glm::ortho(0.0f, (real32)plot->region.right, (real32)plot->region.top, 0.0f));

	//draw the plot with lines and the plot itself
	drawPlotScaleLines(plot->range, plot->region);
	for (auto& transferfunction : RpGui::context->activetransferfunctions) {
		drawTransferFunctionPhase(plot, &transferfunction, &RpGui::context->buffer);
	}

	//start drawing the text
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->fontpipeline2D, { &RpGui::context->font.cdata, 1 });
	RpGui::renderer2D.pushTexture(RpGui::context->font.atlas);
	RpGui::drawPlotScaleValues(plot->range, plot->region, &RpGui::context->font, textscale);

	RpGui::renderer2D.flush({ nullptr, 0 });

	//end renderpass for this display
	plot->endRenderPass();




	RpGui::context->phaseplot.ImGuiDraw();
	RpGui::context->magnitudeplot.ImGuiDraw();

	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	static bool functionpanelopen;
	if (ImGui::Begin("functions")) {
		for (auto& tf : RpGui::context->activetransferfunctions) {
			drawComponent<drawRpConnectionGui>(tf.name, RpGui::context, (void*) & tf);
		}
	} ImGui::End();

	static char pythoncommandbuffer[256];

	ImGui::ShowDemoWindow();

	if (ImGui::Begin("Python Commandwindow")) {
		if (ImGui::InputText("cmd", pythoncommandbuffer, IM_ARRAYSIZE(pythoncommandbuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
			// This code only runs when Enter is pressed
			try {
				//py::initialize_interpreter(&config);
				py::exec(pythoncommandbuffer);
				
			}
			catch (py::error_already_set& e) {
				// This will print the actual Python error message and traceback to C++ stderr
				ERR << "Python Error: " << e.what() << "\n";
			}

			for (uint32 i = 0; i < sizeof(pythoncommandbuffer); i++) {
				pythoncommandbuffer[i] = '\0';
			}
			ImGui::SetKeyboardFocusHere(-1);
		}

		if (ImGui::Button("run exec file")) {
			PH::Platform::FileBuffer b;
			if (PH::Platform::loadFile(&b, "res/exec.py")) {
				INFO << (char*)b.data << "\n\n";
				try {
					py::exec((char*)b.data);
				}
				catch (py::error_already_set& e) {
					ERR << "Python Error: " << e.what() << "\n";
				}

				PH::Platform::unloadFile(&b);
			}
		}

	} ImGui::End();

	static bool statsopen;
	if (ImGui::Begin("stats")) {
		ImGui::Text("framerate %f", 1.0f / Engine::getTimeStep());
		ImGui::Text("mousepos %f, %f", Engine::Events::getMousePos().x, Engine::getParentDisplay()->viewport.y - Engine::Events::getMousePos().y);

		ImGui::Text("amount of global descriptors %u", Engine::Renderer2D::getStats(RpGui::renderer2D.getContext()).useddescriptors);
	} ImGui::End();

	//static bool demowindowopen = true;
	//ImGui::ShowDemoWindow(&demowindowopen);

	PH::Engine::EndDockspace();

	//render the imgui widgetes to the screen
	ImGui::Render();
	PH::Platform::GFX::drawImguiWidgets(ImGui::GetDrawData());



	PH::Engine::endRenderPass(*Engine::getParentDisplay());
	RpGui::renderer2D.end();
	
	return true;
}

PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy) {

	PH::RpGui::INFO << "destroying Rp-Gui application...\n";

	{
		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "PlotViewPanels" << YAML::Value << YAML::BeginMap;
		RpGui::context->magnitudeplot.serialize(out);
		RpGui::context->phaseplot.serialize(out);
		out << YAML::EndMap;

		out << YAML::Key << "CurrentProject" << YAML::Value << RpGui::context->openproject.getC_Str();
		out << YAML::Key << "PythonHome" << YAML::Value << RpGui::context->pythonhome.getC_Str();
		out << YAML::EndMap;

		Engine::FileIO::writeYamlFile(out, "RpGui.ini");
	}

	{
		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "TransferFunctions" << YAML::Value << YAML::BeginSeq;
		for (const auto& t : RpGui::context->activetransferfunctions) {
			RpGui::serializeTransferFunction(t, out);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Engine::FileIO::writeYamlFile(out, RpGui::context->openproject.getC_Str());
	}

	
	//system("PAUSE");
	return true;
}