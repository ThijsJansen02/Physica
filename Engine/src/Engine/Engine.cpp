//include the platform api to have access to platfom functions
#include <Platform/platformAPI.cpp>
#include "Engine.h"
#include "Display.h"
#include "Rendering.h"
#include "Scene.h"
#include "Events.h"
#include <Engine/Display.h>

#define ARENA_ALLOCATOR_SIZE (MEGA_BYTE * 128)

namespace PH::Engine {

	void* imguiAllocator(size_t sz, void* userdata) {
		return PH::Engine::Allocator::alloc(sz);
	}

	void imguiDeallocator(void* data, void* userdata) {
		PH::Engine::Allocator::dealloc(data);
	}

	namespace Intern {
		CONSOLE_WRITE(ConsoleWrite) {
			Platform::consoleWrite(str);
		}
	}

	PH::Base::LogStream<Intern::ConsoleWrite> INFO;
	PH::Base::LogStream<Intern::ConsoleWrite> WARN;
	PH::Base::LogStream<Intern::ConsoleWrite> ERR;

	PH::Base::DynamicMemoryBuffer Engine::Allocator::memory;
	thread_local PH::Base::MemoryArena Engine::ArenaAllocator::arena;

	PH::Platform::GFX::RenderpassDescription createDisplayRenderpass();

	namespace Assets {
		Platform::GFX::DescriptorSetLayout createSceneDescriptorSetLayout();
	}

	struct Context {
		real32 lastframetime_ms;
		real32 timestep_ms;

		Display parentdisplay;
	};

	Engine::Context* context;

	bool32 beginNewFrame(PH::Platform::Context* c) {

		//calculate timing
		real32 currentframetime = PH::Platform::getTimeMs();
		context->timestep_ms = currentframetime - context->lastframetime_ms;
		context->lastframetime_ms = currentframetime;

		//reset final display
		context->parentdisplay.framebuffersize = { c->windowwidth, c->windowheight };
		context->parentdisplay.viewport = { (real32)c->windowwidth, (real32)c->windowheight };

		Engine::Events::startNewFrame();
		return true;
	}

	Display* getParentDisplay() {
		return &context->parentdisplay;
	}

	real32 getTimeStep() {
		return context->timestep_ms / 1000.0f;
	}

	bool32 init(const EngineInitInfo& init) {
		Base::base_log = Intern::ConsoleWrite;

		Engine::Allocator::init(init.memory, init.memorysize);
		Engine::ArenaAllocator::init(Engine::Allocator::alloc(ARENA_ALLOCATOR_SIZE), ARENA_ALLOCATOR_SIZE);

		context = (Engine::Context*)Engine::Allocator::alloc(sizeof(Engine::Context));
		
		//set the timing variables
		context->lastframetime_ms = 0.0;
		context->timestep_ms = 0.0f;

		//setup the main display
		Display display;
		display.colorattachment = PH_GFX_NULL;
		display.depthattachment = PH_GFX_NULL;
		display.viewport = { 0.0f, 0.0f };
		display.framebuffersize = { 0, 0 };
		display.renderpass = 0;
		display.fb = PH_GFX_NULL;

		context->parentdisplay = display;

		//set the Imgui context in this translation unit
		ImGui::SetCurrentContext(init.platformcontext->imguicontext);
		ImGui::SetAllocatorFunctions(imguiAllocator, imguiDeallocator, nullptr);

		//init the display renderpass... might make this optional if the use of displays is not required
		Engine::Display::defaultrenderpassdescription = createDisplayRenderpass();
		Engine::Assets::Scene::descriptorsetlayout = Engine::Assets::createSceneDescriptorSetLayout();

		Engine::Renderer3D::cube = Engine::Renderer3D::createCube();

		return true;
	}

	PH::Platform::GFX::Buffer createDynamicUniformBuffer(sizeptr size) {

		PH::Platform::GFX::BufferCreateinfo ubocreate;
		ubocreate.bufferusage = Platform::GFX::BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		ubocreate.size = size;
		ubocreate.memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		ubocreate.data = nullptr;
		ubocreate.dynamic = true;

		PH::Platform::GFX::Buffer result{};
		PH::Platform::GFX::createBuffers(&ubocreate, &result, 1);

		return result;
	}
}