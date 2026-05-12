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

#include <shobjidl.h>

#include "TransferFunction.h"

#include "Context.h"


#define PY_SSIZE_T_CLEA


#ifdef _DEBUG
	#undef _DEBUG
	#include <Python.h>
	#define _DEBUG
#else
	#include <Python.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/embed.h>

#include "embeddedpython.h"

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

void loadShaders() {

	auto defaultquadvert = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_quadshader.vert", Platform::GFX::SHADER_STAGE_VERTEX_BIT);
	auto defaultquadfrag = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_quadshader.frag", Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

	RpGui::context->pipeline2D = Engine::Renderer2D::createGraphicsPipelineFromBinaries(&RpGui::context->magnitudeplot.display,
		defaultquadvert.getArray(),
		defaultquadfrag.getArray(),
		{ nullptr, 0 }
	);

	Engine::DynamicArray<uint8>::destroy(&defaultquadvert);
	Engine::DynamicArray<uint8>::destroy(&defaultquadfrag);

	auto defaultfontvert = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_fontshader.vert", Platform::GFX::SHADER_STAGE_VERTEX_BIT);
	auto defaultfontfrag = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_fontshader.frag", Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

	RpGui::context->fontpipeline2D = Engine::Renderer2D::createGraphicsPipelineFromBinaries(&RpGui::context->magnitudeplot.display,
		defaultfontvert.getArray(),
		defaultfontfrag.getArray(),
		{ &RpGui::fontuserlayout, 1 }
	);

	Engine::DynamicArray<uint8>::destroy(&defaultfontvert);
	Engine::DynamicArray<uint8>::destroy(&defaultfontfrag);
}

void deserializeApplication() {

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
}

void deserializeProject(const char* projectdir) {

	auto proj = Engine::FileIO::loadYamlfile(projectdir);
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
			tf.connection.commandqueue.push({ Engine::String::create("export PATH=$PATH:/opt/redpitaya/bin;fpgautil -b sinewave_generator_wrapper.bit.bin") });

			for (auto f : tf.filters) {
				sentFilterToRp(f, targetfs, &tf.connection);
			}
		}
	}
}

#define EMBED_PYTHON


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

	//buffer for drawing the plots
	RpGui::context->buffer = Engine::ArrayList<glm::vec2>::create(10);
	RpGui::context->font = RpGui::loadFont("c:/windows/fonts/arial.ttf", 512, 32.0f);

	RpGui::context->plottitle = Engine::String::create("Frequency Response");

	//setup example transferfunctions; should in the future be loaded from a serialized document
	RpGui::context->activetransferfunctions = Engine::ArrayList<TransferFunction>::create(1);


	loadShaders();
	deserializeApplication();

#ifdef EMBED_PYTHON
	initPython(RpGui::context->pythonhome.getC_Str());
#endif



	deserializeProject(RpGui::context->openproject.getC_Str());



	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	


	//init the renderer
	Engine::Renderer2D::InitInfo init{};
	init.currentpipeline = RpGui::context->pipeline2D;
	init.descriptorsetlayouts = { nullptr, 0 };
	init.instancebuffersize = 8 * MEGA_BYTE;
	init.shadowmapdimensions = 0;

	RpGui::renderer2D = Engine::Renderer2D::Wrapper::create(init);
	return true;
}
	//this is going to be the function that draws the plot, it takes in the vertices of the plot, the range of the plot and the region of the plot, and it draws the plot using the renderer2D wrapper, this is going to be called from the drawTransferFunctionMagnitude and drawTransferFunctionPhase functions, which are going to generate the vertices for the plot based on the transfer function and then call this function to draw the plot, this is going to allow us to separate the logic of generating the vertices for the plot from the logic of drawing the plot, which is going to make it easier to maintain and extend in the future, for example if we want to add support for different types of plots or different types of data sources for the plots, we can just generate different vertices for those plots and then call this function to draw them without having to duplicate any code.

typedef void (*UIfunc) (void*, RpGui::Context*, int32&);

template<UIfunc func>
inline void drawComponent(const Engine::String& name, PH::RpGui::Context* context, void* comp, int32& id)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
	
	ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

	
	ImGui::PushID(id++);
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
		func(comp, context, id);
		ImGui::TreePop();
	}

	if (removeComponent) {
			
	}
	ImGui::PopID();
}

real32 dragspeed = 0.002;

void drawRpConnectionGui(void* function, RpGui::Context* context, int32& id) {

	RpGui::TransferFunction* tf = (RpGui::TransferFunction*)function;



	char buffer[256];
	PH::Base::stringCopy(tf->connection.remoteip.getC_Str(), buffer, 256);

	ImGui::PushID(id++);
	if (ImGui::InputText("", buffer, 256)) {
		tf->connection.remoteip.set(buffer);
	}

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
		tf->connection.commandqueue.push({ Engine::String::create("export PATH=$PATH:/opt/redpitaya/bin;fpgautil -b sinewave_generator_wrapper.bit.bin") });
	}
	ImGui::PopID();



	ImGui::PushID(id++);
	PH::Base::stringCopy(tf->currentcommand.getC_Str(), buffer, 256);
	if (ImGui::InputText("##transferfunctionname", buffer, 256)) {
		tf->currentcommand.set(buffer);
	}

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
	ImGui::PopID();

	real64 targetfs = 125000000.0 / 256.0;

	for (auto& f : tf->filters) {
		ImGui::PushID(id++);

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

void drawPlotDataGui(void* function, RpGui::Context* context, int32& id) {
	PlotData* plotdata = (PlotData*)function;
	char buffer[256];
	PH::Base::stringCopy(plotdata->name.getC_Str(), buffer, 256);

	ImGui::PushID(id++);
	if (ImGui::InputText("##plotdataname", buffer, 256)) {
		plotdata->name.set(buffer);
	}
	ImGui::SameLine();
	if (ImGui::Button("remove")) {
		//remove the plot from the openedplots array
		for (uint32 i = 0; i < RpGui::context->openedplots.getCount(); i++) {
			if (&RpGui::context->openedplots[i] == plotdata) {
				RpGui::context->openedplots.remove(i);
				break;
			}
		}
	}

	ImGui::ColorEdit3("color", &plotdata->color.r);
	ImGui::PopID();
}

Engine::String OpenFileDialog()
{
	IFileOpenDialog* pFileOpen = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr,
		CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen));

	if (SUCCEEDED(hr))
	{
		hr = pFileOpen->Show(nullptr);

		if (SUCCEEDED(hr))
		{
			IShellItem* pItem;
			hr = pFileOpen->GetResult(&pItem);

			if (SUCCEEDED(hr))
			{
				PWSTR pszFilePath = nullptr;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

				if (SUCCEEDED(hr))
				{
					std::wstring ws(pszFilePath);
					CoTaskMemFree(pszFilePath);
					pItem->Release();
					pFileOpen->Release();

					std::string cstr = std::string(ws.begin(), ws.end());
					return Engine::String::create(cstr.c_str());
				}
				pItem->Release();
			}
		}
		pFileOpen->Release();
	}

	return Engine::String::create("");
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
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..."))
			{
				Engine::String path = OpenFileDialog();
				if (!path.isEmpty())
				{
					// Use the selected file
					//INFO << path.getC_Str() << " was selected.\n";

					try {
						char buffer[1024];
						sprintf_s(buffer, sizeof(buffer), "OpenCSVandWriteToGUI(r\"%s\")", path.getC_Str());

						INFO << "python command ran: " << buffer << "\n";

						py::exec(buffer);
					}
					catch (py::error_already_set& e) {
						// This will print the actual Python error message and traceback to C++ stderr
						ERR << "Python Error: " << e.what() << "\n";
					}
				}
				Engine::String::destroy(&path);
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


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
	

	for (auto& plotdata : RpGui::context->openedplots) {
		auto copy = Engine::DynamicArray<glm::vec2>::create(plotdata.data.getArray());
		drawPlot(copy.getArray(), plot->range, plot->region, plotdata.color);
		Engine::DynamicArray<glm::vec2>::destroy(&copy);
	}


	//start drawing the text
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->fontpipeline2D, { &RpGui::context->font.cdata, 1 });
	RpGui::renderer2D.pushTexture(RpGui::context->font.atlas);
	RpGui::drawPlotScaleValues(plot->range, plot->region, &RpGui::context->font, textscale);
	drawXlabel(plot->region, &RpGui::context->font, "frequency (Hz)", textscale);
	drawTitle(plot->region, &RpGui::context->font, RpGui::context->plottitle.getC_Str(), 1.0f);
	drawYlabel(plot->region, &RpGui::context->font, "magnitude (dB)", textscale);

	//draw legemd

	real32 y = plot->region.top - 50.0f;
	real32 x = plot->region.right - 300.0f;
	for (auto plotdata : RpGui::context->openedplots) {

		drawText(&RpGui::context->font, plotdata.name.getC_Str(), { x, y }, textscale, plotdata.color);

		y -= 20.0f;
	}


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

	
	for (auto& plotdata : RpGui::context->openedplots) {
		auto copy = Engine::DynamicArray<glm::vec2>::create(plotdata.phasedata.getArray());
		drawPlot(copy.getArray(), plot->range, plot->region, plotdata.color);
		Engine::DynamicArray<glm::vec2>::destroy(&copy);
	}

	//start drawing the text
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->fontpipeline2D, { &RpGui::context->font.cdata, 1 });
	RpGui::renderer2D.pushTexture(RpGui::context->font.atlas);
	RpGui::drawPlotScaleValues(plot->range, plot->region, &RpGui::context->font, textscale);
	drawXlabel(plot->region, &RpGui::context->font, "frequency (Hz)", textscale);
	drawYlabel(plot->region, &RpGui::context->font, "Phase (radians)", textscale);

	RpGui::renderer2D.flush({ nullptr, 0 });

	//end renderpass for this display
	plot->endRenderPass();

	RpGui::context->phaseplot.ImGuiDraw();
	RpGui::context->magnitudeplot.ImGuiDraw();

	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	int32 id = 0;
	static bool functionpanelopen;
	if (ImGui::Begin("functions")) {
		for (auto& tf : RpGui::context->activetransferfunctions) {
			drawComponent<drawRpConnectionGui>(tf.name, RpGui::context, (void*)&tf, id);
		}

		for (auto& PlotData : RpGui::context->openedplots) {
			drawComponent<drawPlotDataGui>(PlotData.name, RpGui::context, (void*)&PlotData, id);
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