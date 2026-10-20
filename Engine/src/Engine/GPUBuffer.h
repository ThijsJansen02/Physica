#pragma once
#include <Platform/platformAPI.h>
namespace PH::Engine {

	template<typename T>
	struct StaticGPUBuffer {

		static StaticGPUBuffer create(Base::Array<T> items, PH::Platform::GFX::BufferUsageFlags bufferusage) {
			StaticGPUBuffer b;
			b.capacity = items.count;

			Platform::GFX::BufferCreateinfo ci;
			ci.data = items.data;
			ci.bufferusage = bufferusage;
			ci.memoryproperties = Platform::GFX::MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			ci.size = items.count * sizeof(T);
			ci.dynamic = false;

			PH::Platform::GFX::createBuffers(&ci, &b.buffer, 1);
		}

		sizeptr capacity;
		PH::Platform::GFX::Buffer buffer;
	};

	template<typename T>
	struct DynamicGPUBuffer {
		static DynamicGPUBuffer create(Base::Array<T> items, PH::Platform::GFX::BufferUsageFlags bufferusage) {
			DynamicGPUBuffer b;
			b.capacity = items.count;
			
			Platform::GFX::BufferCreateinfo ci;
			ci.data = items.data;
			ci.bufferusage = bufferusage;
			ci.memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;
			ci.size = items.count * sizeof(T);
			ci.dynamic = true; 

			PH::Platform::GFX::createBuffers(&ci, &b.buffer, 1);

			return b;
		}

		static DynamicGPUBuffer create(sizeptr count, PH::Platform::GFX::BufferUsageFlags bufferusage) {
			DynamicGPUBuffer b;
			b.capacity = count;

			Platform::GFX::BufferCreateinfo ci;
			ci.data = nullptr;
			ci.bufferusage = bufferusage;
			ci.memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;
			ci.size = count * sizeof(T);
			ci.dynamic = true;

			PH::Platform::GFX::createBuffers(&ci, &b.buffer, 1);

			return b;
		}

		T* map() {
			void* memory = nullptr;

			PH::Platform::GFX::mapBuffer(&memory, buffer);
			return (T*)memory;
		}

		T& operator[](sizeptr index) {
			T* memory = map();
			return memory[index];
		}

		void set(Base::Array<T> data) {
			T* memory = map();
			Base::copyMemory(data.data, memory, data.count * sizeof(T));
		}

		sizeptr capacity;
		PH::Platform::GFX::Buffer buffer;
	};


}