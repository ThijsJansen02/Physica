#pragma once
#include <Platform/platformAPI.h>

#include <Base/Base.h>
#include <Base/random.h>
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
		PH_ENGINE_API CONSOLE_WRITE(ConsoleWrite);
	}

	extern PH_ENGINE_API PH::Base::LogStream<Intern::ConsoleWrite> INFO;
	extern PH_ENGINE_API PH::Base::LogStream<Intern::ConsoleWrite> WARN;
	extern PH_ENGINE_API PH::Base::LogStream<Intern::ConsoleWrite> ERR;

	struct Display;

	class PH_ENGINE_API Allocator {
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

		template<typename T>
		static T* inst(const T& inst) {
			T* result = (T*)malloc(sizeof(T));
			*result = inst;
			return result;
		}
#else
		template<typename T>
		static T* alloc() {
			return (T*)alloc(sizeof(T));
		}

		
		template<typename T>
		static T* push(const T& t) {
			T* inst = (T*)alloc(sizeof(T));
			*inst = t;
			return inst;
		}


		static void* alloc(PH::sizeptr size);
		static PH::bool32 dealloc(void* mem);
		static void init(void* mem, PH::sizeptr size);

		static PH::Base::DynamicMemoryBuffer memory;
#endif
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

	template<typename T>
	using DynamicArray = PH::Base::DynamicArray<T, Engine::Allocator>;

	using String = PH::Base::String<Engine::Allocator>;

	typedef PH::Base::FilePath<Engine::Allocator> FilePath;

	template<typename T>
	using DynamicArray = PH::Base::DynamicArray<T, Engine::Allocator>;

	typedef uint64 UUID;
	
	struct EngineInitInfo {
		PH::Platform::Context* platformcontext;
		sizeptr memorysize;
		void* memory;
	};

	bool32 init(const EngineInitInfo& init);

	//should always be called at the the start of a new frame, updates parameters
	bool32 beginNewFrame(PH::Platform::Context* c);

	//returns the time step from last frame in seconds
	PH_ENGINE_API real32 getTimeStep();
	
	//returns the parent display, the parentdisplay is the display created by the platform which is blitted to the screen
	Display* getParentDisplay();

	inline UUID createRandomUUID() {
		return Base::pcg_hash((uint32)(PH::Platform::getTimeMs() * 1000.0f));
	}

	PH::Platform::GFX::Buffer createDynamicUniformBuffer(sizeptr size);
}