#pragma once
#include <Platform/platformAPI.h>

#include <Base/Base.h>
#include <Base/Memory.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Base/Log.h>

#define ENGINE_USE_MALLOC 0

namespace PH::Engine {


#ifdef PH_ENGINE_EXPORT
	#define PH_ENGINE_API __declspec(dllexport)
#else 
	#define PH_ENGINE_API __declspec(dllimport)
#endif


	namespace Intern {
		CONSOLE_WRITE(ConsoleWrite);
	}

	extern PH::Base::LogStream<Intern::ConsoleWrite> INFO;
	extern PH::Base::LogStream<Intern::ConsoleWrite> WARN;
	extern PH::Base::LogStream<Intern::ConsoleWrite> ERR;

	class Allocator {
	public:

#if ENGINE_USE_MALLOC
		static void* alloc(PH::sizeptr size) {
			return malloc(size);
		}

		static PH::bool32 dealloc(void* mem) {
			free(mem);
			return true;
		}

		static void init(void* mem, PH::sizeptr size) {

		}
#else
		static void* alloc(PH::sizeptr size) {
//			PH_DEBUG_ASSERT(size > 0, "memory of size 0 is pointless!")

			return PH::Base::DynamicAllocateFirstFit(&memory, size);
		}
		static PH::bool32 dealloc(void* mem) {
//			PH_DEBUG_ASSERT(mem, "memory is nullptr")

			return PH::Base::DynamicDeallocate(&memory, mem);
		}

		static void init(void* mem, PH::sizeptr size) {
			memory = PH::Base::createDynamicMemoryBuffer(mem, size);
		}

#endif
		static PH::Base::DynamicMemoryBuffer memory;
	};

	class ArenaAllocator {
	public:
		static void* alloc(PH::sizeptr size) {
			return PH::Base::retrieveMemory(&arena, size);
		}
		static PH::Base::MemoryArenaReset getResetPoint() {
			return PH::Base::getCurrentResetPoint(&arena);
		}
		static PH::bool32 resetArena(PH::Base::MemoryArenaReset resetpoint) {
			return PH::Base::resetMemoryArena(&arena, resetpoint);
		}
		static PH::bool32 dealloc(void* mem) {
			return true;
		}
		static void init(void* mem, PH::sizeptr size) {
			arena = PH::Base::createMemoryArena(mem, size);
		}

		thread_local static PH::Base::MemoryArena arena;
	};

	//simple wrapper class to automatically set the arena scope to the current scope
	class ArenaScope {
	public:
		ArenaScope() {
			resetpoint = ArenaAllocator::getResetPoint();
		}

		~ArenaScope() {
			ArenaAllocator::resetArena(resetpoint);
		}
	private:
		PH::Base::MemoryArenaReset resetpoint;
	};

	template<typename T> 
	using ArrayList = PH::Base::ArrayList<T, Engine::Allocator>;
	using String = PH::Base::String<Engine::Allocator>;

	typedef PH::Base::FilePath<Engine::Allocator> FilePath;

	template<typename T>
	using DynamicArray = PH::Base::DynamicArray<T, Engine::Allocator>;

	typedef uint64 UUID;
	
	struct EngineInitInfo {
		sizeptr memorysize;
		void* memory;
	};

	bool32 init(const EngineInitInfo& init);
	bool32 beginNewFrame();

	real32 getTimeStep();

	PH::Platform::GFX::Buffer createDynamicUniformBuffer(sizeptr size);
}