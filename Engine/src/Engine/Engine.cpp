//include the platform api to have access to platfom functions
#include <Platform/platformAPI.cpp>
#include "Engine.h"
#include "Display.h"
#include "Rendering.h"
#include "Scene.h"
#include "Events.h"

#define ARENA_ALLOCATOR_SIZE (MEGA_BYTE * 128)

namespace PH::Engine {

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

	real32 lastframetime_ms;
	real32 timestep_ms;

	bool32 beginNewFrame() {

		real32 currentframetime = PH::Platform::getTimeMs();
		timestep_ms = currentframetime - lastframetime_ms;
		lastframetime_ms = currentframetime;

		Engine::Events::startNewFrame();

		return true;
	}

	real32 getTimeStep() {
		return timestep_ms / 1000.0f;
	}

	bool32 init(const EngineInitInfo& init) {
		Base::base_log = Intern::ConsoleWrite;

		Engine::Allocator::init(init.memory, init.memorysize);
		Engine::ArenaAllocator::init(Engine::Allocator::alloc(ARENA_ALLOCATOR_SIZE), ARENA_ALLOCATOR_SIZE);

		//init the display renderpass... might make this optional if the use of displays is not required
		Engine::Display::renderpass = createDisplayRenderpass();
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