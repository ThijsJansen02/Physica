#pragma once
#include <Platform/platformAPI.h>
namespace PH::Engine {

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

		static void destroy(DynamicGPUBuffer<T>* b) {
			PH::Platform::GFX::destroyBuffers(&b->buffer, 1);
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
			return b;
		}

		static void destroy(StaticGPUBuffer* buffer) {
			PH::Platform::GFX::destroyBuffers(&buffer->buffer, 1);
		}

		//uploads data to the static buffer by creating a dynamic staging buffer and deleting the staging buffer afterwards
		void uploadData(const T* data, sizeptr count) {

			DynamicGPUBuffer<T> stagingbuffer = DynamicGPUBuffer<T>::create({(T*)data, count}, PH::Platform::GFX::BUFFER_USAGE_TRANSFER_SRC_BIT);
			uploadData(data, count, &stagingbuffer);
			DynamicGPUBuffer<T>::destroy(&stagingbuffer);
		}

		////does not synch to this thread, copies the data in the render commandbuffer, creates a staging buffer and deletes it after the copy
		void uploadDataAsynchronous(const T* data, sizeptr count) {
			DynamicGPUBuffer<T> stagingbuffer = DynamicGPUBuffer<T>::create({(T*)data, count }, PH::Platform::GFX::BUFFER_USAGE_TRANSFER_SRC_BIT);
			uploadDataAsynchronous(data, count, &stagingbuffer);
			DynamicGPUBuffer<T>::destroy(&stagingbuffer);
		}

		//does not synch to this thread, copies the data in the render commandbuffer
		void uploadDataAsynchronous(const T* data, sizeptr count, DynamicGPUBuffer<T>* stagingbuffer) {
			if (count > capacity) {
				count = capacity;
			}

			if (count > stagingbuffer->capacity) {
				count = stagingbuffer->capacity;
			}

			void * stagingbuffermap = stagingbuffer->map();
			Base::copyMemory((void*)data, stagingbuffermap, count * sizeof(T));

			PH::Platform::GFX::copyBufferAsynchronous(stagingbuffer->buffer, this->buffer, count * sizeof(T), 0, 0);
		}

		void uploadData(const T* data, sizeptr count, DynamicGPUBuffer<T>* stagingbuffer) {
			if (count > capacity) {
				count = capacity;
			}

			if (count > stagingbuffer->capacity) {
				count = stagingbuffer->capacity;
			}

			void* stagingbuffermap = stagingbuffer->map();
			Base::copyMemory((void*)data, stagingbuffermap, count * sizeof(T));

			PH::Platform::GFX::copyBuffer(stagingbuffer->buffer, this->buffer, count * sizeof(T), 0, 0);
		}

		sizeptr capacity;
		PH::Platform::GFX::Buffer buffer;
	};
}