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

#include <python.h>
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

namespace ImGui {
	void StyleColorsSpectrum() {
		ImGuiStyle* style = &ImGui::GetStyle();
		style->GrabRounding = 4.0f;

		ImVec4* colors = style->Colors;
		colors[ImGuiCol_Text] = ColorConvertU32ToFloat4(Spectrum::GRAY800); // text on hovered controls is gray900
		colors[ImGuiCol_TextDisabled] = ColorConvertU32ToFloat4(Spectrum::GRAY500);
		colors[ImGuiCol_WindowBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ColorConvertU32ToFloat4(Spectrum::GRAY50); // not sure about this. Note: applies to tooltips too.
		colors[ImGuiCol_Border] = ColorConvertU32ToFloat4(Spectrum::GRAY300);
		colors[ImGuiCol_BorderShadow] = ColorConvertU32ToFloat4(Spectrum::Static::NONE); // We don't want shadows. Ever.
		colors[ImGuiCol_FrameBg] = ColorConvertU32ToFloat4(Spectrum::GRAY75); // this isnt right, spectrum does not do this, but it's a good fallback
		colors[ImGuiCol_FrameBgHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY50);
		colors[ImGuiCol_FrameBgActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
		colors[ImGuiCol_TitleBg] = ColorConvertU32ToFloat4(Spectrum::GRAY300); // those titlebar values are totally made up, spectrum does not have this.
		colors[ImGuiCol_TitleBgActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
		colors[ImGuiCol_TitleBgCollapsed] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
		colors[ImGuiCol_MenuBarBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100);
		colors[ImGuiCol_ScrollbarBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100); // same as regular background
		colors[ImGuiCol_ScrollbarGrab] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
		colors[ImGuiCol_ScrollbarGrabHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
		colors[ImGuiCol_ScrollbarGrabActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
		colors[ImGuiCol_CheckMark] = ColorConvertU32ToFloat4(Spectrum::BLUE500);
		colors[ImGuiCol_SliderGrab] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
		colors[ImGuiCol_SliderGrabActive] = ColorConvertU32ToFloat4(Spectrum::GRAY800);
		colors[ImGuiCol_Button] = ColorConvertU32ToFloat4(Spectrum::GRAY75); // match default button to Spectrum's 'Action Button'.
		colors[ImGuiCol_ButtonHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY50);
		colors[ImGuiCol_ButtonActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
		colors[ImGuiCol_Header] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
		colors[ImGuiCol_HeaderHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE500);
		colors[ImGuiCol_HeaderActive] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
		colors[ImGuiCol_Separator] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
		colors[ImGuiCol_SeparatorHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
		colors[ImGuiCol_SeparatorActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
		colors[ImGuiCol_ResizeGrip] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
		colors[ImGuiCol_ResizeGripHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
		colors[ImGuiCol_ResizeGripActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
		colors[ImGuiCol_PlotLines] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
		colors[ImGuiCol_PlotLinesHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
		colors[ImGuiCol_PlotHistogram] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
		colors[ImGuiCol_PlotHistogramHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
		colors[ImGuiCol_TextSelectedBg] = ColorConvertU32ToFloat4((Spectrum::BLUE400 & 0x00FFFFFF) | 0x33000000);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ColorConvertU32ToFloat4((Spectrum::GRAY900 & 0x00FFFFFF) | 0x0A000000);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
}


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

	examplefunction.filters.pushBack(filter1);
	filter1.cutoff = 4000.0f;
	examplefunction.filters.pushBack(filter1);
	filter1.cutoff = 10000.0f;
	examplefunction.filters.pushBack(filter1);

	return examplefunction;
}

namespace py = pybind11;

//actually usable functions in python!
void setFilterCutoff(int filternumber, float cutoff) {
	RpGui::context->activetransferfunctions[0].filters[filternumber].cutoff = cutoff;

	recalculateFilter(&RpGui::context->activetransferfunctions[0].filters[filternumber]);	
}

//actually usable functions in python!
void setFilterQfactor(int filternumber, float qfactor) {
	RpGui::context->activetransferfunctions[0].filters[filternumber].Qfactor = qfactor;

	recalculateFilter(&RpGui::context->activetransferfunctions[0].filters[filternumber]);
}

// Bind it to a Python module
PYBIND11_EMBEDDED_MODULE(RpGui, m) {
	m.doc() = "C++ functions for my RpGui";
	m.def("setFilterCutoff", &setFilterCutoff, "sets the cutoff of a filter",
		py::arg("a"), py::arg("b"));
	m.def("setFilterQfactor", &setFilterQfactor, "set the qfactor of a filter",
		py::arg("filternumber"), py::arg("qfactor"));
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
	
	
	PyConfig config{};

	// 1. Initialize with default Python configuration
	PyConfig_InitPythonConfig(&config);

	config.isolated = 1;

	//set home
	const wchar_t* pyhome = L"C:/Users/Thijs/OneDrive/Documenten/programming/physica/dep/cpython";
	PyConfig_SetString(&config, &config.home, pyhome);

	PyConfig_SetString(&config, &config.program_name, L"RP-GUI");

	config.module_search_paths_set = 1;
	PyWideStringList_Append(&config.module_search_paths, L"C:/Users/Thijs/OneDrive/Documenten/programming/physica/dep/cpython/Lib");
	PyWideStringList_Append(&config.module_search_paths, L"C:/Users/Thijs/OneDrive/Documenten/programming/physica/dep/cpython/PCbuild/amd64");
	PyWideStringList_Append(&config.module_search_paths, L"C:/Users/Thijs/OneDrive/Documenten/programming/physica/dep/cpython/Lib/site-packages");


	try {
		py::initialize_interpreter(&config);
		py::exec(R"(
			import sys
			print(f"Hello world", flush=True)
			print(f"version {sys.version}")
		)");
	}
	catch (py::error_already_set& e) {
		// This will print the actual Python error message and traceback to C++ stderr
		ERR << "Python Error: " << e.what() << "\n";
	}

	

	/*
	PyStatus s;
	s = Py_InitializeFromConfig(&config);
	if (PyStatus_Exception(s)) {
		ERR << s.err_msg << "\n";
	}

	PyConfig_Clear(&config);

	PyRun_SimpleString("print(\"hello world!\")");
	*/
	ssh_init(); //libssh test

	RpGui::context = (RpGui::Context*)Engine::Allocator::alloc(sizeof(RpGui::Context));

	auto ini = Engine::FileIO::loadYamlfile("RpGui.ini");

	RpGui::context->magnitudeplot = RpGui::PlotViewPanel::create({ -10.0f, -10.0f, 10.0f, 10.0f }, "magnitude");
	RpGui::context->phaseplot = RpGui::PlotViewPanel::create({ -10.0f, -180.0f, 10.0f, 180.0f }, "phase");

	//lock the xaxis for both plots together
	RpGui::context->magnitudeplot.xlock = &RpGui::context->phaseplot;
	RpGui::context->phaseplot.xlock = &RpGui::context->magnitudeplot;

	RpGui::context->openproject = Engine::String::create("project1.rpproj");

	//loading the application settings
	if (ini) {

		const auto& plotviewpanels = ini["PlotViewPanels"];
		if (plotviewpanels) {
			RpGui::context->magnitudeplot.deserialize(plotviewpanels["magnitude"]);
			RpGui::context->phaseplot.deserialize(plotviewpanels["phase"]);
		}


		auto currentprojectdir = ini["CurrentProject"];
		if (currentprojectdir) {
			RpGui::context->openproject.set(currentprojectdir.as<Engine::String>());
		}
	}

	RpGui::context->font = RpGui::loadFont("c:/windows/fonts/arial.ttf", 512, 32.0f);
	RpGui::context->pipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(&RpGui::context->magnitudeplot.display, "res/shaders/default_quadshader.vert", "res/shaders/default_quadshader.frag", {nullptr, 0});
	RpGui::context->fontpipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(&RpGui::context->magnitudeplot.display, "res/shaders/default_fontshader.vert", "res/shaders/default_fontshader.frag", { &RpGui::fontuserlayout, 1 });
	
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

	
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
	float lineHeight = 18.0f;
	bool open = ImGui::TreeNodeEx((void*)PH::Base::uint32Hash((uint32)name.getChar(0)), treeNodeFlags, name.getC_Str());
	ImGui::PopStyleVar();


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


#define M_PI 3.14159265359

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
		if (ImGui::DragFloat("Cutoff", &f.cutoff, f.cutoff * dragspeed)) {
			recalculateFilter(&f);

			BiQuadCoefficients dcoeffs = bilinearTransform(getBandPassBiquadCoefficientsContinuous(prewarp(2 * M_PI * f.cutoff, targetfs), f.Qfactor), targetfs);

			Engine::String rpcommand = generateRPfilterString(dcoeffs);

			if (tf->connection.open) {
				tf->connection.commandqueue.push({ rpcommand });
				ReleaseSemaphore(tf->connection.semaphore, 1, nullptr);
			}

		}
		if(ImGui::DragFloat("Q factor", &f.Qfactor, f.Qfactor * dragspeed)) {
			recalculateFilter(&f);

			BiQuadCoefficients dcoeffs = bilinearTransform(getBandPassBiquadCoefficientsContinuous(prewarp(2 * M_PI * f.cutoff, targetfs), f.Qfactor), targetfs);

			Engine::String rpcommand = generateRPfilterString(dcoeffs);

			if (tf->connection.open) {
				tf->connection.commandqueue.push({ rpcommand });
				ReleaseSemaphore(tf->connection.semaphore, 1, nullptr);
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

		out << YAML::Key << "CurrentProject" << YAML::Value << RpGui::context->openproject.getC_Str();

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