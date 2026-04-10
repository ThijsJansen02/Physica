#include "vulkanAPI.h"

namespace PH::Vulkan {

	uint32 findMemoryType(VkPhysicalDevice physicaldevice, uint32 typefilter, VkMemoryPropertyFlags properties) {

		VkPhysicalDeviceMemoryProperties memproperties;
		vkGetPhysicalDeviceMemoryProperties(physicaldevice, &memproperties);

		for (uint32_t i = 0; i < memproperties.memoryTypeCount; i++) {
			if ((typefilter & (1 << i)) && (memproperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		ERR << "Failed to find memorytype!\n";
		return 0xFFFFFFFF;
	}

	bool32 createBuffers(
		VulkanAppContext* context,
		PH::Platform::GFX::BufferCreateinfo* createinfos,
		PH::Platform::GFX::Buffer* buffers,
		PH::uint32 count
	) {

		for (uint32 i = 0; i < count; i++) {

			//creating the buffer object
			PH::Platform::GFX::BufferCreateinfo& createinfo = createinfos[i];
			Buffer buffer;
			buffer.isdynamic = createinfo.dynamic;

			VkBufferCreateInfo vkcreateinfo{};
			vkcreateinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

			vkcreateinfo.size = createinfo.size;
			vkcreateinfo.usage = convertBufferUsage(createinfo.bufferusage);
			vkcreateinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			buffer.usage = vkcreateinfo.usage;

			//if data is going to be copied by this function to the buffer, usage should have the propertie VK_BUFFER_USAGE_TRANSFER_DST
			if (createinfo.data) {
				if (!Platform::GFX::hasflag(createinfo.memoryproperties,
					Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT)) {

					vkcreateinfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				}
			}

			//is set to sharing mode exclusive because we do not share the buffer between queues. If we want to use multiple queues in the future we'll have to change it.
			VkBuffer vkbuffer;
			VkDeviceMemory vkmemory;

			//a non-dynamic/static buffer just uses one underlying vulkan buffer
			if (!createinfo.dynamic) {
				if (vkCreateBuffer(context->device, &vkcreateinfo, nullptr, &vkbuffer) != VK_SUCCESS) {
					ERR << "failed to create buffer!\n";
					return false;
				}

				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					buffer.buffer[i] = vkbuffer;
				}
			}
			else {

				//if its a dynamic buffer a seperate buffer is created for every frame so we can write to one buffer while it is being used by the graphics card to draw the other frame in flight
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					if (vkCreateBuffer(context->device, &vkcreateinfo, nullptr, &vkbuffer) != VK_SUCCESS) {
						ERR << "failed to create buffer!\n";
						return false;
					}
					buffer.buffer[i] = vkbuffer;
				}
			}

			//get the memeory requirements for this buffer from the specification of the buffer.
			VkMemoryRequirements memrequirements{};
			vkGetBufferMemoryRequirements(context->device, vkbuffer, &memrequirements);

			//allocate memory for the buffer. if the buffer is dynamic memory is allocated for #MAX_FRAMES_IN_FLIGHT buffers
			VkMemoryAllocateInfo allocinfo{};
			allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocinfo.memoryTypeIndex = findMemoryType(context->physicaldevice, memrequirements.memoryTypeBits, convertMemoryPropertyflags(createinfo.memoryproperties));

			if (!createinfo.dynamic) {
				allocinfo.allocationSize = memrequirements.size;
				if(vkAllocateMemory(context->device, &allocinfo, nullptr, &vkmemory) != VK_SUCCESS) {
					ERR << "failed to allocate memory!\n";
					PH_DEBUG_BREAK();
					return false;
				}
				vkBindBufferMemory(context->device, vkbuffer, vkmemory, 0);
			}
			else {
				//when the buffer is dynamic we still allocate one block of memory because this is cheaper 
				allocinfo.allocationSize = memrequirements.size * MAX_FRAMES_IN_FLIGHT;
				sizeptr offset = 0;
				if (vkAllocateMemory(context->device, &allocinfo, nullptr, &vkmemory) != VK_SUCCESS) {
					ERR << "failed to allocate memory!\n";
					PH_DEBUG_BREAK();
					return false;
				}

				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					vkBindBufferMemory(context->device, buffer.buffer[i], vkmemory, offset);
					offset += memrequirements.size;
				}
			}

			buffer.memory = vkmemory;
			buffer.buffersize = createinfo.size;
			bool32 memcopied = false;

			//if the buffers are layd out in a host visisble and coherent fashion we can persistently map the memory to local adresses
			if (Platform::GFX::hasflag(createinfo.memoryproperties, 
				Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
				Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT
			))
				{
				if (!createinfo.dynamic) {
					void* data;
					vkMapMemory(context->device, buffer.memory, 0, buffer.buffersize, 0, &data);
					for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
						buffer.memorymaps[i] = data;
					}
					if (createinfo.data) {
						Base::copyMemory(createinfo.data, data, createinfo.size);
					}
				}
				else {
					void* data;
					vkMapMemory(context->device, buffer.memory, 0, memrequirements.size * MAX_FRAMES_IN_FLIGHT, 0, &data);
					
					for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
						buffer.memorymaps[i] = data;
						if (createinfo.data) {
							Base::copyMemory(createinfo.data, data, createinfo.size);
						}
						data = (uint8*)data + memrequirements.size;
					}
				}

			}

			//add the buffer to the data structure
			buffers[i] = context->buffers.add(buffer);

			//if there is data in the createinfo but it is not host visible, a staging buffer is created on the fly and used to copy the data into the buffer
			//check weather there is data and the data is not already copied
			if (createinfo.data && !Platform::GFX::hasflag(createinfo.memoryproperties,
				Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT)) 
			{

				Platform::GFX::BufferCreateinfo stagingbuffercreate{};
				stagingbuffercreate.bufferusage = Platform::GFX::BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingbuffercreate.dynamic = false;
				stagingbuffercreate.memoryproperties = {
					Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT
				};

				stagingbuffercreate.size = createinfo.size;
				stagingbuffercreate.data = createinfo.data;
				
				Platform::GFX::Buffer stagingbuffer;
				createBuffers(&stagingbuffercreate, &stagingbuffer, 1);
				copyBuffer(stagingbuffer, buffers[i], createinfo.size, 0, 0);

				destroyBuffers(&stagingbuffer, 1);
			}

		}
		return true;
	}

	PH_GFX_CREATE_BUFFERS(createBuffers) {
		return createBuffers(context_s, createinfos, buffers, count);
	}

	bool32 destroyBuffers(VulkanAppContext* context, Platform::GFX::Buffer* buffers, uint32 count) {
		for (uint32 i = 0; i < count; i++) {
			Buffer buffer = context->buffers[buffers[i]];
			vkFreeMemory(context->device, buffer.memory, nullptr);

			//if the buffer is dynamic all sub buffers need to be destroyed;
			if (buffer.isdynamic) {
				for (uint32 j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
					vkDestroyBuffer(context->device, buffer.buffer[j], nullptr);
				}
			}
			else {
				vkDestroyBuffer(context->device, buffer.buffer[0], nullptr);
			}
			context->buffers.remove(buffers[i]);
		}
		return true;
	}

	PH_GFX_DESTROY_BUFFERS(destroyBuffers) {
		return destroyBuffers(context_s, buffers, count);
	}

#define MAX_BOUND_BUFFERS 16

	PH_GFX_BIND_VERTEXBUFFERS(bindVertexBuffers) {

		VkBuffer vkbuffers[MAX_BOUND_BUFFERS];

		for (uint32 i = 0; i < nbindings; i++) {
			vkbuffers[i] = context_s->buffers[buffers[i]].buffer[context_s->frameindex];
		}

		vkCmdBindVertexBuffers(context_s->commandbuffers[context_s->frameindex], firstbinding, nbindings, vkbuffers, offsets);

		return true;
	}

	PH_GFX_BIND_INDEXBUFFER(bindIndexBuffer) {

		VkBuffer vkbuffer = context_s->buffers[buffer].buffer[context_s->frameindex];

		vkCmdBindIndexBuffer(context_s->commandbuffers[context_s->frameindex], vkbuffer, offset, VK_INDEX_TYPE_UINT32);
		return true;
	}
	
	bool32 copyBuffer(VulkanAppContext* context, Platform::GFX::Buffer srcbuffer, Platform::GFX::Buffer dstbuffer, sizeptr size, sizeptr srcoffset, sizeptr dstoffset) {

		VkBuffer vksrcbuffer = context->buffers[srcbuffer].buffer[context->frameindex];
		VkBuffer vkdstbuffer = context->buffers[dstbuffer].buffer[context->frameindex];

		VkCommandBuffer commandbuffer = beginSingleTimeCommands(context);

		VkBufferCopy buffercopy{};
		buffercopy.dstOffset = dstoffset;
		buffercopy.srcOffset = srcoffset;
		buffercopy.size = size;
		vkCmdCopyBuffer(commandbuffer, vksrcbuffer, vkdstbuffer, 1, &buffercopy);

		endSingleTimeCommands(context, commandbuffer);

		return true;	
	}

	PH_GFX_COPY_BUFFER(copyBuffer) {
		return copyBuffer(context_s, srcbuffer, dstbuffer, size, srcoffset, dstoffset);
	}

	PH::bool32 mapBuffer(VulkanAppContext* context, void** mmap, PH::Platform::GFX::Buffer buffer) {
		Buffer vkbuffer = context->buffers[buffer];
		PH_DEBUG_ASSERT(vkbuffer.isdynamic, "buffer is not dynamic");

		*mmap = vkbuffer.memorymaps[context->frameindex];
		return true;
	}

	PH_GFX_MAP_BUFFER(mapBuffer) {
		return mapBuffer(context_s, memory, buffer);
	}

	PH::bool32 unmapBuffer(VulkanAppContext* context, Platform::GFX::Buffer buffer) {
		return true;
	}

	PH_GFX_UNMAP_BUFFER(unmapBuffer) {
		return unmapBuffer(context_s, buffer);
	}

}