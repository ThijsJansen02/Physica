#include <Windows.h>

#include "Base/Memory.h"
#include "Base/Datastructures/String.h"
#include "Base/Log.h"
#include <Base/Datastructures/Array.h>
#include <Base/Memory.h>

#include "Platform/platformAPI.h"
#include "Platform/vulkan/vulkan.h"
#include "Platform/vulkan/vulkanAPI.h"
#include "Platform/Events.h"

//#define _CRT_SECURE_NO_WARNINGS

//for implementing imgui on the system
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_win32.cpp"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_vulkan.cpp"

/// <summary>
/// global handles to the input and output stream of the console
/// </summary>
static HANDLE hStdout, hStdin;
static CONSOLE_SCREEN_BUFFER_INFO csbiInfo;


/// <summary>
/// typedefs for the application API imported functions
/// </summary>
typedef PH_APPLICATION_UPDATE(applicationUpdateFN);
typedef PH_APPLICATION_INITIALIZE(applicationInitializeFN);
typedef PH_APPLICATION_DESTROY(applicationDestroyFN);
typedef bool applicationReloadFN(PH::Platform::Intern::PlatformAPI* platform);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define PLATFORM_USE_MALLOC 0

/// <summary>
/// The allocator for all platform related memory allocations. just uses malloc for now.
/// </summary>
class PlatformAllocator {
public:

#if PLATFORM_USE_MALLOC

	static void* alloc(PH::sizeptr size) {
		return malloc((size_t)size);
	}


	static PH::bool32 dealloc(void* mem) {
		free(mem);
		return true;
	}

	static PH::bool32 dealloc(void* mem, PH::sizeptr size) {
		free(mem);
		return true;
	}
#else
	static void* alloc(PH::sizeptr size) {
		return PH::Base::DynamicAllocateFirstFit(&buffer, size);
	}


	static PH::bool32 dealloc(void* mem) {
		PH::Base::DynamicDeallocate(&buffer, mem);
		return true;
	}

	static PH::bool32 dealloc(void* mem, PH::sizeptr size) {
		PH::Base::DynamicDeallocate(&buffer, mem);
		return true;
	}
#endif
	static PH::Base::DynamicMemoryBuffer buffer;
};

PH::Base::DynamicMemoryBuffer PlatformAllocator::buffer;


typedef PH::Base::String<PlatformAllocator> win32String;

template<typename type>
using win32DynamicArray = PH::Base::DynamicArray<type, PlatformAllocator>;

template<typename type>
using win32Arraylist = PH::Base::ArrayList<type, PlatformAllocator>;

/// <summary>
/// writes the contents of str to the hstdout handle. which has a console attached to it.
/// </summary>
/// <param name="str">The string that is written to the console</param>
void win32_consoleWrite(const PH::Base::SubString& str) {
	DWORD byteswritten = 0;
	WriteFile(hStdout, str.getC_Str(), (DWORD)str.getLength(), &byteswritten, NULL);
}

void win32_consoleWrite_dummy(const PH::Base::SubString& str) {
	DWORD byteswritten = 0;
	//WriteFile(hStdout, str.getC_Str(), (DWORD)str.getLength(), &byteswritten, NULL);
}

void win32_consoleWrite(const win32String& str) {
	win32_consoleWrite(str.getSubString());
}


PH::Base::LogStream<win32_consoleWrite> INFO;
PH::Base::LogStream<win32_consoleWrite> WARN;
PH::Base::LogStream<win32_consoleWrite> ERR;

PH::uint64 pack(PH::uint32 high, PH::uint32 low) {

	PH::uint64 result = (PH::uint64)high;
	result = result << 32;
	result += low;
	return result;
}

/// <summary>
/// KEY_DOWN: lparem(keycode)
/// KEY_UP: lparam(keycode)
/// MOUSE_MOVE: lparam(x), rparam(y)
/// </summary>
/// <param name="message"></param>
/// <returns></returns>
PH::Platform::Event win32_translateMessage(MSG message) {

	PH::Platform::Event result{};
	result.type = PH_EVENT_TYPE_NULL;

	switch (message.message) {

	case WM_MOUSEWHEEL:
		result.type = PH_EVENT_TYPE_MOUSE_SCROLLED;
		result.lparam = (PH::uint64)(void*)GET_WHEEL_DELTA_WPARAM(message.wParam);
		break;
	case WM_MOUSEMOVE:
		result.type = PH_EVENT_TYPE_MOUSE_MOVED;
		result.lparam = LOWORD(message.lParam);
		result.rparam = HIWORD(message.lParam);
		break;
	case WM_KEYDOWN:
		result.type = PH_EVENT_TYPE_KEY_PRESSED;
		result.lparam = message.wParam;
		break;
	case WM_KEYUP:
		result.type = PH_EVENT_TYPE_KEY_RELEASED;
		result.lparam = message.wParam;
		break;
	case WM_CHAR:
		result.type = PH_EVENT_TYPE_CHAR;
		result.lparam = message.wParam;
		break;

	case WM_LBUTTONDOWN:
		result.type = PH_EVENT_TYPE_MOUSEBUTTON_PRESSED;
		result.lparam = PH_LMBUTTON;
		break;
	case WM_RBUTTONDOWN:
		result.type = PH_EVENT_TYPE_MOUSEBUTTON_PRESSED;
		result.lparam = PH_RMBUTTON;
		break;
	case WM_MBUTTONDOWN:
		result.type = PH_EVENT_TYPE_MOUSEBUTTON_PRESSED;
		result.lparam = PH_MBUTTON;
		break;
	case WM_LBUTTONUP:
		result.type = PH_EVENT_TYPE_MOUSEBUTTON_RELEASED;
		result.lparam = PH_LMBUTTON;
		break;
	case WM_RBUTTONUP:
		result.type = PH_EVENT_TYPE_MOUSEBUTTON_RELEASED;
		result.lparam = PH_RMBUTTON;
		break;
	case WM_MBUTTONUP:
		result.type = PH_EVENT_TYPE_MOUSEBUTTON_RELEASED;
		result.lparam = PH_MBUTTON;
		break;

	}
	return result;	
}

/// <summary>
/// initializes the global console handles
/// </summary>
/// <returns></returns>
bool win32_initializeConsole() {

	AllocConsole();

	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	//reopen the standard out for the crt 
	FILE* fDummy;
	// Redirect standard output to the console
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	// Redirect standard error to the console
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	// Redirect standard input to the console
	freopen_s(&fDummy, "CONIN$", "r", stdin);

	// Optional: Disable buffering to see output immediately
	setvbuf(stdout, NULL, _IONBF, 0);

	if (hStdin == INVALID_HANDLE_VALUE ||
		hStdout == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, TEXT("GetStdHandle"), TEXT("Console Error"),
			MB_OK);
		return false;
	}

	//save the current text colors
	if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
	{
		MessageBox(NULL, TEXT("GetConsoleScreenBufferInfo"),
			TEXT("Console Error"), MB_OK);
		return false;
	}

	/*
	if (!SetConsoleTextAttribute(hStdout, FOREGROUND_RED |
		FOREGROUND_INTENSITY))
	{
		MessageBox(NULL, TEXT("SetConsoleTextAttribute"),
			TEXT("Console Error"), MB_OK);
		return false;
	}*/
	return true;
}

/// <summary>
/// initializes the a window and returns the handle in hwnd.
/// </summary>
/// <param name="hwnd">a pointer to the handle of the window that is initialized</param>
/// <param name="hInstance">the hInstance</param>
/// <param name="nCmdShow">the cmdshow</param>
/// <returns></returns>
bool win32_initializeWindow(HWND* hwnd, HINSTANCE hInstance, int nCmdShow, void* userptr) {

	const wchar_t CLASS_NAME[] = L"Main Window Class";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);
	*hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		userptr        // Additional application data
	);

	if (hwnd == NULL)
	{
		return false;
	}

	ShowWindow(*hwnd, nCmdShow);
	return true;
}

void win32_freeFile(PH::Platform::FileBuffer* file) {
	if (file->data) {
		VirtualFree(file->data, 0, MEM_RELEASE);
	}
	file->data = nullptr;
	file->size = 0;
}

PH::Platform::FileBuffer win32_readFile(const char* filepath) {

	PH::Platform::FileBuffer result = {};
	HANDLE filehandle = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (filehandle == INVALID_HANDLE_VALUE) {
		ERR << "invalid filehandle!\nfilepath: " << filepath << "\n";
		return result;
	}

	LARGE_INTEGER filesize;
	if (!GetFileSizeEx(filehandle, &filesize)) {
		CloseHandle(filehandle);
		ERR << "failed to get file size!\nfilepath: " << filepath << "\n";
		return result;
	}

	result.size = (PH::uint64)filesize.QuadPart;
	result.data = VirtualAlloc(0, filesize.QuadPart, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	DWORD bytesread;
	if (!ReadFile(filehandle, result.data, filesize.QuadPart, &bytesread, 0)) {

		DWORD error = GetLastError();

		CloseHandle(filehandle);
		win32_freeFile(&result);
		return result;
	}

	CloseHandle(filehandle);
	return result;
}

PH::bool32 win32_writeFile(const PH::Platform::FileBuffer& file, const char* filepath) {

	HANDLE filehandle = CreateFileA(filepath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (filehandle == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD nbyteswritten;
	if (!WriteFile(filehandle, file.data, file.size , &nbyteswritten, 0)) {
		CloseHandle(filehandle);
		return false;
	}

	CloseHandle(filehandle);
	return true;
}

PH_LIST_DIRECTORIES(win32_listFiles) {

	char* writeptr = buffer;
	PH::sizeptr returnsize = 0;

	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA ffd;
	DWORD dwError = 0;

	auto dirstring = PH::Base::String<PlatformAllocator>::create(filepath);
	dirstring.replace('/', '\\');
	dirstring.append("\\*");

	hFind = FindFirstFileA(dirstring.getC_Str(), &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		return 0;
	}

	do
	{
		PH::sizeptr namelength = PH::Base::stringLength(ffd.cFileName);
		if (buffersize > (namelength + 1) && buffer != nullptr)
		{
			if (writeptr != buffer) {
				*(writeptr - 1) = ';';
			}
			writeptr += PH::Base::stringCopy(ffd.cFileName, writeptr, buffersize);
			buffersize -= namelength + 1;
		}
		else {
			buffersize = 0;
		}
		returnsize += namelength + 1;

	} while (FindNextFileA(hFind, &ffd) != 0);

	PH::Base::String<PlatformAllocator>::destroy(&dirstring);
	return returnsize;
}

#define GRAPHICS_MEMORY_SIZE (8 * MEGA_BYTE)
#define PLATFORM_MEMORY_SIZE (8 * MEGA_BYTE)

PH_LOAD_FILE(win32_loadFile) {
	*file = win32_readFile(filepath);
	return file->data != nullptr;
}
PH_UNLOAD_FILE(win32_unloadFile) {
	win32_freeFile(file);
	return true;
}

struct win32PlatformState {
	PH::bool32 running;
	PH::uint32 windowwidth;
	PH::uint32 windowheight;

	win32Arraylist<PH::Platform::Event> eventlist;
};

static void vkImGuiCheckResult(VkResult err) {
	if (err != VK_SUCCESS) {
		DebugBreak();
		ERR << "a vulkan call resulted in an error!";
		return;
	}
}

void* imguiAllocator(size_t sz, void* userdata) {
	return PlatformAllocator::alloc(sz);
}

void imguiDeallocator(void* data, void* userdata) {
	PlatformAllocator::dealloc(data);
}


void initImGui(HWND windowhandle, const PH::Vulkan::VulkanAppContext& vulkancontext) {

	IMGUI_CHECKVERSION();

	ImGui::SetAllocatorFunctions(imguiAllocator, imguiDeallocator, nullptr);
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.BackendFlags &= ~ImGuiBackendFlags_PlatformHasViewports;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowhandle);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vulkancontext.instance;
	init_info.PhysicalDevice = vulkancontext.physicaldevice;
	init_info.Device = vulkancontext.device;
	init_info.QueueFamily = vulkancontext.queuefamily;
	init_info.Queue = vulkancontext.graphicsqueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = vulkancontext.descriptorpool;

	init_info.PipelineInfoMain.RenderPass = vulkancontext.renderpasses[vulkancontext.finalrenderpass].renderpass;
	init_info.PipelineInfoMain.Subpass = 0;
	init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	init_info.MinImageCount = 2;
	init_info.ImageCount = vulkancontext.swapchainimages.getCapacity();
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = vkImGuiCheckResult;

	//imGui_ImplVulkan_LoadFunctions();

	ImGui_ImplVulkan_Init(&init_info);
}

//draws the imgui widgets to a vulkan context
//TODO(Thijs): Move to vulkan API or other implementation file
PH_GFX_DRAW_IMGUI_WIDGETS(vulkan_draw_imgui_widgets) {
	ImGui_ImplVulkan_RenderDrawData(drawdata, PH::Vulkan::context_s->commandbuffers[PH::Vulkan::context_s->frameindex]);
	return true;
}

PH_GFX_CREATE_IMGUI_IMAGE(vulkan_create_imgui_image) {
	PH::Vulkan::TextureImage vktexture = PH::Vulkan::context_s->textureimages[texture];
	return (ImTextureID)ImGui_ImplVulkan_AddTexture(vktexture.sampler, vktexture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

LARGE_INTEGER systemstartcount;
LARGE_INTEGER PerformanceFrequency;

PH_GET_TIME_MS(win32_getTimeMs) {

	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return 1000.0f*(PH::real64)(count.QuadPart - systemstartcount.QuadPart) / (PH::real64)(PerformanceFrequency.QuadPart);
}

struct win32_ThreadInfo {
	PH::Platform::ThreadCreateInfo info;
};

//weird implementation, why does it have to create a memory arena for the GFX layer?
DWORD WINAPI win32_ThreadProc(
	_In_ LPVOID lpParameter
) {
	win32_ThreadInfo* threadinfo = (win32_ThreadInfo*)lpParameter;
	if (threadinfo->info.usegfx) {
		PH::sizeptr memorysize = threadinfo->info.threadworkmemorysize;
		PH::Vulkan::ArenaAllocator::arena = PH::Base::createMemoryArena(PlatformAllocator::alloc(memorysize), memorysize);
	}
	threadinfo->info.threadproc(threadinfo->info.userdata);
	return 0;
}


struct win32_Thread {
	HANDLE handle;
	DWORD threadid;

	win32_ThreadInfo info;
};


PH::Base::ArrayList<win32_Thread*, PlatformAllocator> openthreads;

PH_CREATE_THREAD(win32_createThread) {

	win32_Thread* win32thread = openthreads.pushBack((win32_Thread*)PlatformAllocator::alloc(sizeof(win32_Thread)));
	win32thread->info.info = info;

	win32thread->handle = CreateThread(nullptr, 0, win32_ThreadProc, &win32thread->info, 0, (DWORD*) & win32thread->threadid);
	thread->handle = win32thread->handle;
	thread->id = win32thread->threadid;

	return true;
}


struct Application {
	applicationUpdateFN* applicationUpdate;
	applicationDestroyFN* applicationDestroy;
	applicationInitializeFN* applicationInitialize;
	applicationReloadFN* applicationReload;
};

PH::bool32 win32_loadApplication(HWND windowhandle, const char* dllname, Application* app) {

	//loading the application into the platform
	HMODULE applicationlib = LoadLibraryA(dllname);
	if (applicationlib == nullptr) {
		ERR << "failed to load application\n";
		PH_DEBUG_BREAK();
		DestroyWindow(windowhandle);
		return 1;
	}

	applicationUpdateFN* applicationUpdate = (applicationUpdateFN*)GetProcAddress(applicationlib, "applicationUpdate");
	if (!applicationUpdate) { ERR << "failed to load application update\n"; return false; }

	applicationDestroyFN* applicationDestroy = (applicationUpdateFN*)GetProcAddress(applicationlib, "applicationDestroy");
	if (!applicationDestroy) { ERR << "failed to load application Destroy\n"; return false; }

	applicationInitializeFN* applicationInitialize = (applicationInitializeFN*)GetProcAddress(applicationlib, "applicationInitialize");
	if (!applicationInitialize) { ERR << "failed to load application Initialize\n"; return false; }

	applicationReloadFN* applicationReload = (applicationReloadFN*)GetProcAddress(applicationlib, "applicationReload");
	if (!applicationReload) { ERR << "failed to load application reload\n"; return false; }

	app->applicationDestroy = applicationDestroy;
	app->applicationUpdate = applicationUpdate;
	app->applicationReload = applicationReload;
	app->applicationInitialize = applicationInitialize;

	return true;
}

PH_DLL_EXPORT bool applicationReload(PH::Platform::Intern::PlatformAPI* platform);

struct PlatformAppContext {
	HWND windowhandle;
} platformcontext;

PH_OPEN_FILE_DIALOG(win32_openFileDialog) {

	OPENFILENAMEA ofn;
	CHAR szFile[1024] = { 0 };
	CHAR currentDir[256] = { 0 };

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = platformcontext.windowhandle;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = info.filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn)) {
		char* str = ofn.lpstrFile;
		while (*str) {
			if (*str == '\\') {
				*str = '/';
			}
			str++;
		}
		return PH::Base::stringCopy(ofn.lpstrFile, info.resultbuffer, info.resultbuffersize);
	}

	return false;

}

PH::uint32 lastwindowwidth;
PH::uint32 lastwindowheight;

//main file, the start of the program. contains the game loop, input handling, application loading and window creation.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	PlatformAllocator::buffer = PH::Base::createDynamicMemoryBuffer(VirtualAlloc(nullptr, PLATFORM_MEMORY_SIZE, MEM_COMMIT, PAGE_READWRITE), PLATFORM_MEMORY_SIZE);
	
	openthreads = PH::Base::ArrayList<win32_Thread*, PlatformAllocator>::create(0);

	win32PlatformState state;
	state.running = true;
	state.eventlist = win32Arraylist<PH::Platform::Event>::create(32);

	if (!win32_initializeConsole()) {
		return 1;
	}

	PH::Base::base_log = win32_consoleWrite;

	printf("this is a test print!\n");

	HWND windowhandle;
	if (!win32_initializeWindow(&windowhandle, hInstance, nCmdShow, &state)) {
		return 1;
	}

	RECT windowrect;
	GetClientRect(
		windowhandle,
		&windowrect
	);

	state.windowwidth = windowrect.right - windowrect.left;
	state.windowheight = windowrect.bottom - windowrect.top;

	lastwindowwidth = state.windowwidth;
	lastwindowheight = state.windowheight;
 
	//vulkan init info to set up the vulkancontext.
	PH::Vulkan::VulkanInitInfo initinfo{};
	initinfo.windowwidth = state.windowwidth;
	initinfo.windowheight = state.windowheight;

#ifdef PH_DEBUG
	initinfo.debugactive = true;
#else
	initinfo.debugactive = false;
#endif
	initinfo.memory = VirtualAlloc(nullptr, GRAPHICS_MEMORY_SIZE, MEM_COMMIT, PAGE_READWRITE);
	initinfo.memorysize = GRAPHICS_MEMORY_SIZE;

	//init vulkan with initinfo
	PH::Vulkan::VulkanAppContext vulkancontext{};
	PH::Vulkan::win32_initVulkan(&vulkancontext, initinfo, windowhandle, hInstance);
	PH::Vulkan::context_s = &vulkancontext;

	initImGui(windowhandle, vulkancontext);

	//loading the application;
	Application app;
	if (!win32_loadApplication(windowhandle, "RP-GUI.dll", &app)) {
		PH_DEBUG_BREAK();
	}


	//load the vulkan api into the platform GFX api
	PH::Platform::Intern::PlatformAPI api;
	PH::Vulkan::loadAPI(&api);
	api.loadfile = win32_loadFile;
	api.unloadfile = win32_unloadFile;
	api.writefile = win32_writeFile;

	api.openfiledialog = win32_openFileDialog;
	api.listfiles = win32_listFiles;

	api.gettimems = win32_getTimeMs;

	api.createThread = win32_createThread;

	api.gfxdrawimguiwidgets = vulkan_draw_imgui_widgets;
	api.gfxcreateimguiimage = vulkan_create_imgui_image;

	app.applicationReload(&api);
	applicationReload(&api);


	QueryPerformanceCounter(&systemstartcount);
	QueryPerformanceFrequency(&PerformanceFrequency);

	PH::Platform::Context context;
	context.events = state.eventlist.getArray();
	context.windowwidth = state.windowwidth;
	context.windowheight = state.windowheight;
	context.imguicontext = ImGui::GetCurrentContext();
	
	context.appmemsize = GIGA_BYTE;
	context.appmemory = VirtualAlloc(nullptr, context.appmemsize, MEM_COMMIT, PAGE_READWRITE);
	
	context.ellapsedtimeseconds = 0.0f;

	app.applicationInitialize(context);


	while (state.running)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();


		//query window events and push them back on the platform independent eventqueue.
		MSG msg = { };
		while (PeekMessageA(&msg, windowhandle, 0, 0, PM_REMOVE) > 0) {

			TranslateMessage(&msg);
			PH::Platform::Event e = win32_translateMessage(msg);
			if (e.type == PH_EVENT_TYPE_NULL) {
				DispatchMessage(&msg);
			}
			else {
				state.eventlist.pushBack(e);
				DispatchMessage(&msg);
			}
		}

		//check if window is resized!
		if (lastwindowwidth != state.windowwidth || lastwindowheight != state.windowheight) {
			
			PH::Platform::Event e;
			e.type = PH_EVENT_TYPE_WINDOW_RESIZED;
			e.lparam = pack(state.windowwidth, state.windowheight);
			state.eventlist.pushBack(e);
			
			PH::Vulkan::win32_resizeSwapchain(&vulkancontext, state.windowwidth, state.windowheight);

			lastwindowwidth = state.windowwidth;
			lastwindowheight = state.windowheight;
		}

		
		ImGui::NewFrame();

		PH::Vulkan::startFrame(&vulkancontext);

		LARGE_INTEGER currentframecount;
		QueryPerformanceCounter(&currentframecount);

		//run the application
		context.events = state.eventlist.getArray();
		context.windowwidth = state.windowwidth;
		context.windowheight = state.windowheight;
		context.ellapsedtimeseconds = (PH::real64)(currentframecount.QuadPart - systemstartcount.QuadPart) / (PH::real64)(PerformanceFrequency.QuadPart);

		app.applicationUpdate(context);

		PH::Vulkan::submitFrame(&vulkancontext);

		state.eventlist.clear();
	}

	context.events = state.eventlist.getArray();
	context.windowwidth = state.windowwidth;
	context.windowheight = state.windowheight;
	app.applicationDestroy(context);

	return 0;
}

inline win32PlatformState* getPlatformState(HWND hwnd)
{
	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	win32PlatformState* pState = reinterpret_cast<win32PlatformState*>(ptr);
	return pState;
}

//watch out, this function is on a different thread!
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
		return true;
	}

	if (uMsg == WM_CREATE)
	{
		win32PlatformState* pState;
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pState = reinterpret_cast<win32PlatformState*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pState);
	}
	
	switch (uMsg)
	{
	case WM_DESTROY:
		{
			win32PlatformState* state = getPlatformState(hwnd);
			state->running = false;
			PostQuitMessage(0);
			return 0;

		}
	case WM_SIZE:
		{
			//WINDOW_RESIZE: high(lparam) = width, low(lparam) = height;
			win32PlatformState* state = getPlatformState(hwnd);

			PH::uint32 width = LOWORD(lParam);
			PH::uint32 height = HIWORD(lParam);

			state->windowwidth = width;
			state->windowheight = height;

			return 0;
		}
	}


	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

