#pragma once
#include <vulkan/vulkan.hpp>
#include <Base/Base.h>
#include <Base/Datastructures/Array.h>
#include <Base/Datastructures/Set.h>
#include <Base/Memory.h>
#include <Base/Datastructures/String.h>
#include <Base/Datastructures/FracturedArrayList.h>
#include <Windows.h>
#include "Platform/platformAPI.h"
#include <Base/Log.h>

void win32_consoleWrite(const PH::Base::SubString& str);

namespace PH::Vulkan {

#define VULKAN_USE_MALLOC 0
#define MAX_FRAMES_IN_FLIGHT 2
#define VULKAN_DESCRIPTOR_POOL_SIZE 100;

	extern PH::Base::LogStream<win32_consoleWrite> ERR;
	extern PH::Base::LogStream<win32_consoleWrite> WARN;
	extern PH::Base::LogStream<win32_consoleWrite> INFO;

	class Allocator {
	public:

#if VULKAN_USE_MALLOC
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
			//INFO << "allocated " << (uint32)size << " bytes\n";

			return PH::Base::DynamicAllocateFirstFit(&memory, size);
		}
		static PH::bool32 dealloc(void* mem) {
			//INFO << "memory deallocated!\n";

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

		static thread_local PH::Base::MemoryArena arena;
	};

	template<typename type>
	using FracturedArraylist = PH::Base::FracturedArrayList<type, Allocator>;

	template<typename type>
	using ArrayList = PH::Base::ArrayList<type, Allocator>;

	template<typename type>
	using DynamicArray = PH::Base::DynamicArray<type, Allocator>;

	template<typename type>
	using Array = PH::Base::Array<type>;

	using String = PH::Base::String<Allocator>;
	using SubString = PH::Base::SubString;

	template<typename type>
	using ArraySet = PH::Base::Set<type, ArrayList<type>>;

	struct VulkanInitInfo {
		void* memory;
		sizeptr memorysize;
		bool32 debugactive;

		uint32 windowwidth;
		uint32 windowheight;
	};

	struct DescriptorSetLayout {
		volatile VkDescriptorSetLayout layout;
	};

	struct DescriptorSet {
		bool32 isdynamic;
		VkDescriptorSet set[MAX_FRAMES_IN_FLIGHT];
	};

	struct Renderpass {
		VkRenderPass renderpass;
	};

	struct Shader {
		VkShaderModule vkshadermodule;
		VkShaderStageFlags shaderstage;
	};

	struct GraphicsPipeline {
		volatile VkPipeline vkpipeline;
		VkPipelineLayout layout;
	};

	struct Framebuffer {
		VkFramebuffer buffer;
	};

	struct Buffer {

		VkBuffer buffer[MAX_FRAMES_IN_FLIGHT];
		void* memorymaps[MAX_FRAMES_IN_FLIGHT];
		VkDeviceMemory memory;

		bool32 isdynamic;

		sizeptr buffersize;
		VkBufferUsageFlags usage;
	};

	struct TextureImage {

		VkImage image;
		VkImageView view;
		VkSampler sampler;
		VkDeviceMemory imagememory;
	};

	struct VulkanAppContext {

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugmessenger;
		VkPhysicalDevice physicaldevice;
		VkDevice device;
		VkSurfaceKHR surface;

		VkQueue presentqueue;
		VkQueue graphicsqueue;

		uint32 queuefamily;

		uint32 windowwidth;
		uint32 windowheight;

		VkDescriptorPool descriptorpool;

		VkPipeline pipeline;
		VkPipelineLayout pipelinelayout;
		Platform::GFX::RenderpassDescription finalrenderpass;

		VkSwapchainKHR swapchain;

		DynamicArray<VkImageView> swapchainimageviews;
		DynamicArray<VkImage> swapchainimages;
		DynamicArray<VkFramebuffer> swapchainframebuffers;

		VkFormat swapchainimageformat;
		VkExtent2D swapchainextent;

		VkCommandPool commandpool;
		DynamicArray<VkCommandBuffer> commandbuffers;

		//synchronisation primitives
		DynamicArray<VkSemaphore> imageAvailableSemaphores;
		DynamicArray<VkSemaphore> renderFinishedSemaphores;
		DynamicArray<VkFence> inFlightFences;

		//the frameindex that were currently rendering to;
		uint32 frameindex = 0;
		uint32 imageindex = 0;

		PH::Platform::GFX::GraphicsPipeline boundpipeline;
		///api related objects;
		/// 
		///
		FracturedArraylist<Renderpass> renderpasses;
		FracturedArraylist<Shader> shaders;
		FracturedArraylist<GraphicsPipeline> graphicspipelines;
		FracturedArraylist<Framebuffer> framebuffers;
		FracturedArraylist<Buffer> buffers;
		FracturedArraylist<DescriptorSetLayout> descriptorsetlayouts;
		FracturedArraylist<DescriptorSet> descriptorsets;
		FracturedArraylist<TextureImage> textureimages;

		//for descriptor set allocation synchronization
		Base::mutex_lock descriptorpoollock;
		Base::mutex_lock commandpoollock;
	};

	VkCommandBuffer beginSingleTimeCommands(VulkanAppContext* context);
	void endSingleTimeCommands(VulkanAppContext* context, VkCommandBuffer commandBuffer);

	void win32_initVulkan(VulkanAppContext* context, const VulkanInitInfo& info, HWND hwnd, HINSTANCE instance);
	void startFrame(VulkanAppContext* context);
	void submitFrame(VulkanAppContext* context);
	void win32_resizeSwapchain(VulkanAppContext* context, uint32 width, uint32 height);

}
