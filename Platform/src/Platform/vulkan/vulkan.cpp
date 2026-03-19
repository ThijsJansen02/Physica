#define NOMINMAX
#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan.h"

#include "Base/utils.h"
#include <Base/Memory.h>
#include <Base/Datastructures/Array.h>
#include <Base/Datastructures/String.h>

#include <Base/base.h>
#include "Base/Datastructures/Set.h"
#include "Platform/platformAPI.h"

PH::Platform::FileBuffer win32_readFile(const char* filepath);

namespace PH::Vulkan {

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);


	PH::Base::LogStream<win32_consoleWrite> ERR;
	PH::Base::LogStream<win32_consoleWrite> WARN;
	PH::Base::LogStream<win32_consoleWrite> INFO;

	PH::Base::DynamicMemoryBuffer Allocator::memory;
	thread_local PH::Base::MemoryArena ArenaAllocator::arena;

	PH::bool32 checkValidationLayerSupport(Array<char*> validationlayers) {
		PH::uint32 layercount;
		vkEnumerateInstanceLayerProperties(&layercount, nullptr);

		auto availablelayers = DynamicArray<VkLayerProperties>::create(layercount);
		vkEnumerateInstanceLayerProperties(&layercount, availablelayers.raw());

		for (char* str : validationlayers) {

			bool32 found = false;
			for (const VkLayerProperties& prop : availablelayers) {
				if (PH::Base::stringCompare(str, prop.layerName)) {
					found = true;
				}
			}

			if (!found) {
				WARN << "Layer: " << str << " is not present!";
				availablelayers.release();
				return false;
			}

		}

		availablelayers.release();
		return true;
	}

	VkInstance createInstance(const VulkanInitInfo& info) {

		VkInstance instance;
		VkApplicationInfo appinfo{};
		appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appinfo.pApplicationName = "Physica Application";
		appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appinfo.pEngineName = "Physica engine";
		appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appinfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createinfo.pApplicationInfo = &appinfo;

		auto extensions = ArrayList<char*>::create(3);
		extensions.pushBack("VK_KHR_surface");
		extensions.pushBack("VK_KHR_win32_surface");
		if (info.debugactive) {
			extensions.pushBack("VK_EXT_debug_utils");
		}

		createinfo.ppEnabledExtensionNames = extensions.raw();
		createinfo.enabledExtensionCount = (uint32)extensions.getCount();

		auto requiredvalidationlayers = ArrayList<char*>::create(3);
		if (info.debugactive) {
			requiredvalidationlayers.pushBack("VK_LAYER_KHRONOS_validation");
		}


		if (!checkValidationLayerSupport(requiredvalidationlayers.getArray())) {
			ERR << "couldn't create vulkan instance because a validation layer was not present\n";
		}

		createinfo.enabledLayerCount = (uint32)requiredvalidationlayers.getCount();
		createinfo.ppEnabledLayerNames = requiredvalidationlayers.raw();

		if (vkCreateInstance(&createinfo, nullptr, &instance) != VK_SUCCESS) {
			ERR << "failed to create Vulkan Instance\n";
		}

		requiredvalidationlayers.release();
		extensions.release();

		return instance;
	}

	VkSurfaceFormatKHR chooseSurfaceFormat(Array<VkSurfaceFormatKHR> availableformats) {
		for (const auto& format : availableformats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}

		return availableformats[0];
	}

	VkPresentModeKHR choosePresentMode(Array<VkPresentModeKHR> availablemodes) {
		return VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& mode : availablemodes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}
	}

	VkExtent2D chooseSwapChainExtend(const VkSurfaceCapabilitiesKHR& capabilities, uint32 windowwidth, uint32 windowheight) {
		if (capabilities.currentExtent.width != 0xFFFFFFFF) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualextend = { windowwidth, windowheight };

			actualextend.width = PH::Base::clamp(actualextend.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualextend.height = PH::Base::clamp(actualextend.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualextend;
		}
	}


	VkSurfaceKHR createWin32Surface(VulkanAppContext* context, HWND hwnd, HINSTANCE instance) {
		VkSurfaceKHR surface;
		VkWin32SurfaceCreateInfoKHR createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createinfo.hwnd = hwnd;
		createinfo.hinstance = instance;
		if (vkCreateWin32SurfaceKHR(context->instance, &createinfo, nullptr, &surface) != VK_SUCCESS) {
			ERR << "failed to create the surface!\n";
		}
		return surface;
	}

	void setupDebugMessenger(VulkanAppContext* context) {

		VkDebugUtilsMessengerCreateInfoEXT createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createinfo.pfnUserCallback = &debugCallback;
		createinfo.pUserData = nullptr; // Optional

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");

		if (!func) {
			WARN << "failed to load debugmessenger!\n";
			return;
		}

		func(context->instance, &createinfo, nullptr, &context->debugmessenger);
	}

	bool32 findQueueFamilies(uint32* graphicsfamily, uint32* presentfamily, VkPhysicalDevice device, VkSurfaceKHR surface) {
		uint32 queuefamilycount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, nullptr);

		auto queuefamilies = DynamicArray<VkQueueFamilyProperties>::create(queuefamilycount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, queuefamilies.raw());

		bool graphicsfound = false;
		bool presentfound = false;

		int32 i = 0;
		for (const auto& familyproperties : queuefamilies) {


			if (familyproperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				*graphicsfamily = i;
				graphicsfound = true;
			}

			VkBool32 supported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
			if (supported) {
				*presentfamily = i;
				presentfound = true;
			}

			if (presentfound && graphicsfound) {
				break;
			}

			i++;
		}

		queuefamilies.release();
		return presentfound && graphicsfound;
	}

	bool32 checkDeviceExtensionSupport(VkPhysicalDevice device, Array<const char*> extensions) {
		uint32 availableextensioncount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableextensioncount, nullptr);

		auto availableextensions = DynamicArray<VkExtensionProperties>::create(availableextensioncount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableextensioncount, availableextensions.raw());

		bool32 result = true;
		for (const char* ext : extensions) {
			bool32 found = false;
			for (const VkExtensionProperties& props : availableextensions) {
				if (Base::stringCompare(props.extensionName, ext)) {
					found = true;
					break;
				}
			}
			if (!found) {
				result = false;
				break;
			}
		}
		availableextensions.release();
		return result;
	}

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		DynamicArray<VkSurfaceFormatKHR> formats;
		DynamicArray<VkPresentModeKHR> presentmodes;
	};

	SwapChainSupportDetails querySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapChainSupportDetails result;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &result.capabilities);

		uint32 formatcount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatcount, nullptr);
		result.formats = DynamicArray<VkSurfaceFormatKHR>::create(formatcount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatcount, result.formats.raw());

		uint32 presentmodecount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentmodecount, nullptr);
		result.presentmodes = DynamicArray<VkPresentModeKHR>::create(presentmodecount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentmodecount, result.presentmodes.raw());

		return result;
	}

	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, Array<const char*> deviceextensions) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		INFO << properties.deviceName << "\n";

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);

		uint32 presentfamily = 0;
		uint32 graphicsfamily = 0;

		bool32 queuefamiliessupported = findQueueFamilies(&graphicsfamily, &presentfamily, device, surface);
		bool32 extensionssupported = checkDeviceExtensionSupport(device, deviceextensions);

		bool32 swapchainadequate = false;
		if (extensionssupported) {
			SwapChainSupportDetails swapchaindetails = querySwapChainSupportDetails(device, surface);
			swapchainadequate = !swapchaindetails.formats.empty() && !swapchaindetails.presentmodes.empty();

			swapchaindetails.formats.release();
			swapchaindetails.presentmodes.release();
		}

		//we want a discrete gpu but if there is no discrete gpu we will take whatever we can get as long as it supports the features we need
		//TODO: we should probably make the device selection a bit more sophisticated and not just check if it is a discrete gpu or not but also check for other features and properties that might be important for our application
		//return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.samplerAnisotropy && swapchainadequate && extensionssupported && queuefamiliessupported;
		return properties.deviceType == features.samplerAnisotropy && swapchainadequate && extensionssupported && queuefamiliessupported;
	}

	VkPhysicalDevice pickPhysicalDevice(VulkanAppContext* context) {

		VkPhysicalDevice physicaldevice = VK_NULL_HANDLE;
		uint32 devicecount = 0;

		vkEnumeratePhysicalDevices(context->instance, &devicecount, nullptr);

		if (devicecount <= 0) {
			ERR << "failed to find a vulkan capable GPU!\n";
		}

		auto physicaldevices = DynamicArray<VkPhysicalDevice>::create(devicecount);

		vkEnumeratePhysicalDevices(context->instance, &devicecount, physicaldevices.raw());

		auto deviceextensions = ArrayList<const char*>::create(1);
		deviceextensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		for (auto device : physicaldevices) {
			if (isDeviceSuitable(device, context->surface, deviceextensions.getArray())) {
				physicaldevice = device;
				break;
			}
		}

		if (physicaldevice == VK_NULL_HANDLE) {
			ERR << "failed to find a suitable GPU!\n";
		}


		deviceextensions.release();
		physicaldevices.release();
		return physicaldevice;
	}

	VkDevice createlogicalDevice(VulkanAppContext* context) {

		VkDevice result;

		uint32 graphicsfamily = 0;
		uint32 presentfamily = 0;

		findQueueFamilies(&graphicsfamily, &presentfamily, context->physicaldevice, context->surface);

		auto queuefamilies = ArraySet<uint32>::create(2);
		queuefamilies.add(graphicsfamily);
		queuefamilies.add(presentfamily);

		auto queuecreateinfos = ArrayList<VkDeviceQueueCreateInfo>::create(2);

		real32 queuepriority = 1.0f;
		for (uint32 familyindex : queuefamilies) {
			VkDeviceQueueCreateInfo queuecreateinfo{};
			queuecreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

			queuecreateinfo.pQueuePriorities = &queuepriority;
			queuecreateinfo.queueCount = 1;
			queuecreateinfo.queueFamilyIndex = familyindex;

			queuecreateinfos.pushBack(queuecreateinfo);
		}

		VkPhysicalDeviceFeatures devicefeatures{};
		devicefeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createinfo.pQueueCreateInfos = queuecreateinfos.raw();
		createinfo.queueCreateInfoCount = (uint32)queuecreateinfos.getCount();

		createinfo.pEnabledFeatures = &devicefeatures;

		auto deviceextensions = ArrayList<const char*>::create(1);
		deviceextensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		createinfo.enabledExtensionCount = deviceextensions.getCount();
		createinfo.ppEnabledExtensionNames = deviceextensions.raw();


		if (vkCreateDevice(context->physicaldevice, &createinfo, nullptr, &result) != VK_SUCCESS) {
			ERR << "failed to create logical device!\n";
		}

		context->queuefamily = graphicsfamily;

		vkGetDeviceQueue(result, presentfamily, 0, &context->presentqueue);
		vkGetDeviceQueue(result, graphicsfamily, 0, &context->graphicsqueue);

		queuecreateinfos.release();
		queuefamilies.release();

		return result;
	}

	VkSwapchainKHR createSwapChain(VulkanAppContext* context) {

		VkSwapchainKHR result;

		SwapChainSupportDetails swapchainsupport = querySwapChainSupportDetails(context->physicaldevice, context->surface);

		VkSurfaceFormatKHR surfaceformat = chooseSurfaceFormat(swapchainsupport.formats.getArray());
		VkPresentModeKHR presentmode = choosePresentMode(swapchainsupport.presentmodes.getArray());

		VkExtent2D swapchainextend = chooseSwapChainExtend(swapchainsupport.capabilities, context->windowwidth, context->windowheight);

		uint32 imagecount = swapchainsupport.capabilities.minImageCount + 1;
		if (swapchainsupport.capabilities.maxImageCount > 0) {
			imagecount = Base::minVal(imagecount, swapchainsupport.capabilities.maxImageCount);
		}

		VkSwapchainCreateInfoKHR createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createinfo.surface = context->surface;
		createinfo.minImageCount = imagecount;
		createinfo.imageFormat = surfaceformat.format;
		createinfo.imageColorSpace = surfaceformat.colorSpace;
		createinfo.imageExtent = swapchainextend;
		createinfo.imageArrayLayers = 1;
		createinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32 queuefamilies[2];
		findQueueFamilies(&queuefamilies[0], &queuefamilies[1], context->physicaldevice, context->surface);
		if (queuefamilies[0] != queuefamilies[1]) {
			createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createinfo.queueFamilyIndexCount = ARRAY_LENGTH(queuefamilies);
			createinfo.pQueueFamilyIndices = queuefamilies;
		}
		else {
			createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createinfo.queueFamilyIndexCount = 0;
			createinfo.pQueueFamilyIndices = nullptr;
		}

		createinfo.preTransform = swapchainsupport.capabilities.currentTransform;
		createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createinfo.presentMode = presentmode;
		createinfo.clipped = true;

		createinfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(context->device, &createinfo, nullptr, &result) != VK_SUCCESS) {
			ERR << "failed to create swapchain!\n";
		}

		context->swapchainextent = swapchainextend;
		context->swapchainimageformat = surfaceformat.format;

		return result;
	}

	/// <summary>
	/// creates an array with the swapchain images in it.
	/// </summary>
	/// <param name="context"></param>
	/// <returns></returns>
	DynamicArray<VkImage> getSwapChainImages(VulkanAppContext* context) {

		uint32 imagecount;
		vkGetSwapchainImagesKHR(context->device, context->swapchain, &imagecount, nullptr);

		auto images = DynamicArray<VkImage>::create((sizeptr)imagecount);
		vkGetSwapchainImagesKHR(context->device, context->swapchain, &imagecount, images.raw());

		return images;
	}

	DynamicArray<VkImageView> createSwapChainImageViews(VulkanAppContext* context) {

		uint32 count = 0;
		auto result = DynamicArray<VkImageView>::create(context->swapchainimages.getCapacity());
		for (VkImage im : context->swapchainimages) {
			
			VkImageViewCreateInfo createinfo{};
			createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createinfo.image = im;
			createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createinfo.format = context->swapchainimageformat;

			createinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createinfo.subresourceRange.baseMipLevel = 0;
			createinfo.subresourceRange.levelCount = 1;
			createinfo.subresourceRange.baseArrayLayer = 0;
			createinfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(context->device, &createinfo, nullptr, &result[count]) != VK_SUCCESS) {
				ERR << "failed to create image view for image: " << count << "\n";
			}
			count++;
		}

		return result;
	}

	void cleanUpSwapchain(VulkanAppContext* context) {
		for (VkFramebuffer buffer : context->swapchainframebuffers) {
			vkDestroyFramebuffer(context->device, buffer, nullptr);
		}

		for (VkImageView view : context->swapchainimageviews) {
			vkDestroyImageView(context->device, view, nullptr);
		}

		vkDestroySwapchainKHR(context->device, context->swapchain, nullptr);
	}

	Renderpass createFinalRenderpass(VulkanAppContext* context) {

		VkAttachmentDescription colorattachment{};
		colorattachment.format = context->swapchainimageformat;
		colorattachment.samples = VK_SAMPLE_COUNT_1_BIT;

		colorattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorattachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorattachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


		VkAttachmentReference colorattachmentref{};
		colorattachmentref.attachment = 0;
		colorattachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//this dependency makes the the renderpass wait with the image transition until the image is available 
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; //the subpass that should wait
		dependency.dstSubpass = 0; //the first subpass: colorattachment

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorattachmentref;

		VkRenderPass vkrenderpass;

		VkRenderPassCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createinfo.dependencyCount = 1;
		createinfo.pDependencies = &dependency;

		createinfo.attachmentCount = 1;
		createinfo.pAttachments = &colorattachment;
		createinfo.subpassCount = 1;
		createinfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(context->device, &createinfo, nullptr, &vkrenderpass) != VK_SUCCESS) {
			ERR << "failed to create renderpass! \n";
		}

		Renderpass result{};
		result.renderpass = vkrenderpass;
		return result;
	}

	DynamicArray<VkFramebuffer> createSwapchainFramebuffers(VulkanAppContext* context) {

		auto result = DynamicArray<VkFramebuffer>::create(context->swapchainimageviews.getCapacity());

		VkRenderPass renderpass = context->renderpasses[context->finalrenderpass].renderpass;

		for (uint32 i = 0; i < result.getCapacity(); i++) {

			VkImageView attachments[] = {
				context->swapchainimageviews[i]
			};

			VkFramebufferCreateInfo createinfo{};
			createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

			createinfo.attachmentCount = ARRAY_LENGTH(attachments);
			createinfo.pAttachments = attachments;
			createinfo.renderPass = renderpass;
			createinfo.width = context->swapchainextent.width;
			createinfo.height = context->swapchainextent.height;
			createinfo.layers = 1;

			if (vkCreateFramebuffer(context->device, &createinfo, nullptr, &result[i]) != VK_SUCCESS) {
				ERR << "failed to create framebuffers! \n";
				PH_DEBUG_BREAK();
			}
		}

		return result;
	}

	VkCommandPool createCommandPool(VulkanAppContext* context) {

		VkCommandPool result;

		uint32 graphicsfamily = 0;
		uint32 presentfamily = 0;

		findQueueFamilies(&graphicsfamily, &presentfamily, context->physicaldevice, context->surface);

		VkCommandPoolCreateInfo poolinfo{};
		poolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolinfo.queueFamilyIndex = graphicsfamily;

		if (vkCreateCommandPool(context->device, &poolinfo, nullptr, &result) != VK_SUCCESS) {
			ERR << "failed to create command pool!\n";
			PH_DEBUG_BREAK();
		}

		return result;
	}

	DynamicArray<VkCommandBuffer> createCommandBuffers(VulkanAppContext* context) {
		
		auto result = DynamicArray<VkCommandBuffer>::create(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = context->commandpool;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocinfo.commandBufferCount = result.getCapacity();

		if (vkAllocateCommandBuffers(context->device, &allocinfo, result.raw()) != VK_SUCCESS) {
			ERR << "failed to allocate command buffers!\n";
			PH_DEBUG_BREAK();
		}

		return result;	
	}

	VkDescriptorPool createDescriptorPool(VulkanAppContext* context) {

		VkDescriptorPoolSize poolsize{};
		poolsize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolsize.descriptorCount = 100;

		VkDescriptorPoolSize imagepoolsize{};
		imagepoolsize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imagepoolsize.descriptorCount = 200;

		VkDescriptorPoolSize poolsizes[2] = {
			poolsize, imagepoolsize
		};

		VkDescriptorPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createinfo.poolSizeCount = 2;
		createinfo.pPoolSizes = &poolsizes[0];
		createinfo.maxSets = VULKAN_DESCRIPTOR_POOL_SIZE;

		VkDescriptorPool result;

		if (vkCreateDescriptorPool(context->device, &createinfo, nullptr, &result) != VK_SUCCESS) {
			ERR << "failed to create descriptorpool";
			PH_DEBUG_BREAK();
		}
		return result;
	}

	void beginMainCommandBuffer(VulkanAppContext* context, VkCommandBuffer buffer, uint32 imageindex) {

		/*VkRenderPass renderpass = context->renderpasses[context->finalrenderpass].renderpass;

		VkRenderPassBeginInfo renderpassinfo{};
		renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassinfo.renderPass = renderpass;
		renderpassinfo.framebuffer = context->framebuffers[imageindex];

		renderpassinfo.renderArea.offset = { 0, 0 };
		renderpassinfo.renderArea.extent = context->swapchainextent;

		VkClearValue clearcolor = { {{1.0, 0.0, 0.0, 1.0f}} };
		renderpassinfo.clearValueCount = 1;
		renderpassinfo.pClearValues = &clearcolor;

		vkCmdBeginRenderPass(buffer, &renderpassinfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(context->swapchainextent.width);
		viewport.height = static_cast<float>(context->swapchainextent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = context->swapchainextent;
		vkCmdSetScissor(buffer, 0, 1, &scissor);

		vkCmdDraw(buffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(buffer);*/
	}

	DynamicArray<VkSemaphore> createSemaphores(VulkanAppContext* context) {

		auto result = DynamicArray<VkSemaphore>::create(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreinfo{};
		semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (uint32 i = 0; i < result.getCapacity(); i++) {
			if (vkCreateSemaphore(context->device, &semaphoreinfo, nullptr, &result[i]) != VK_SUCCESS) {
				ERR << "Failed to create semaphore!\n";
			}
		}

		return result;
	}

	DynamicArray<VkFence> createFences(VulkanAppContext* context) {

		auto result = DynamicArray<VkFence>::create(MAX_FRAMES_IN_FLIGHT);

		VkFenceCreateInfo fenceinfo{};
		fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32 i = 0; i < result.getCapacity(); i++) {
			if (vkCreateFence(context->device, &fenceinfo, nullptr, &result[i]) != VK_SUCCESS) {
				ERR << "Failed to create fence!\n";
			}
		}

		return result;
	}

	//starts a commandbuffer for a single time use command set
	VkCommandBuffer beginSingleTimeCommands(VulkanAppContext* context) {

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = context->commandpool;
		allocInfo.commandBufferCount = 1;

		Base::lock(context->commandpoollock);

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer);

		_ReadWriteBarrier();
		Base::unlock(context->commandpoollock);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	//starts ends a single time usage commandbuffer.
	void endSingleTimeCommands(VulkanAppContext* context, VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkFence fence;

		VkFenceCreateInfo fencecreate{};
		fencecreate.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fencecreate.flags = 0;

		if (vkCreateFence(context->device, &fencecreate, nullptr, &fence) != VK_SUCCESS) {
			PH_DEBUG_BREAK();
			WARN << "Failed to create fence!\n";
		}

		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &commandBuffer;
	

		vkQueueSubmit(context->graphicsqueue, 1, &submitinfo, fence);
		vkWaitForFences(context->device, 1, &fence, VK_TRUE, UINT64_MAX);

		Base::lock(context->commandpoollock);
		vkFreeCommandBuffers(context->device, context->commandpool, 1, &commandBuffer);
		_ReadWriteBarrier();
		Base::unlock(context->commandpoollock);

		vkDestroyFence(context->device, fence, nullptr);

	}

#define ARENA_SIZE 32 * KILO_BYTE

	void win32_initVulkan(
		VulkanAppContext* context, 
		const VulkanInitInfo& init, 
		HWND hwnd, 
		HINSTANCE instance
	) {

		Allocator::init(init.memory, init.memorysize);
		ArenaAllocator::init(Allocator::alloc(ARENA_SIZE), ARENA_SIZE);
		
		//initialize storage for renderprimitives
		context->renderpasses = FracturedArraylist<Renderpass>::create(1);
		context->shaders = FracturedArraylist<Shader>::create(1);
		context->graphicspipelines = FracturedArraylist<GraphicsPipeline>::create(1);
		context->buffers = FracturedArraylist<Buffer>::create(1);
		context->descriptorsetlayouts = FracturedArraylist<DescriptorSetLayout>::create(1);
		context->descriptorsets = FracturedArraylist<DescriptorSet>::create(1);
		context->textureimages = FracturedArraylist<TextureImage>::create(1);
		context->framebuffers = FracturedArraylist<Framebuffer>::create(1);

		//TODO(Thijs): add default base types at index 0;
		context->textureimages.add({});
		context->graphicspipelines.add({});
		context->buffers.add({});
		context->framebuffers.add({});
		context->shaders.add({});
		context->descriptorsetlayouts.add({});

		context->windowwidth = init.windowwidth;
		context->windowheight = init.windowheight;

		context->instance = createInstance(init);
		context->surface = createWin32Surface(context, hwnd, instance);

		if (init.debugactive) {
			setupDebugMessenger(context);
		}

		context->physicaldevice = pickPhysicalDevice(context);
		context->device = createlogicalDevice(context);

		context->swapchain = createSwapChain(context);
		context->swapchainimages = getSwapChainImages(context);
		context->swapchainimageviews = createSwapChainImageViews(context);

		context->finalrenderpass = context->renderpasses.add(createFinalRenderpass(context));

		context->swapchainframebuffers = createSwapchainFramebuffers(context);

		context->commandpool = createCommandPool(context);
		context->commandbuffers = createCommandBuffers(context);

		context->inFlightFences = createFences(context);
		context->imageAvailableSemaphores = createSemaphores(context);
		context->renderFinishedSemaphores = createSemaphores(context);

		context->descriptorpool = createDescriptorPool(context);
	}

	void win32_resizeSwapchain(VulkanAppContext* context, uint32 width, uint32 height) {

		if (context->windowheight != height || context->windowwidth != width) {
			vkDeviceWaitIdle(context->device);

			cleanUpSwapchain(context);

			context->windowwidth = width;
			context->windowheight = height;

			context->swapchain = createSwapChain(context);
			context->swapchainimages = getSwapChainImages(context);
			context->swapchainimageviews = createSwapChainImageViews(context);

			context->swapchainframebuffers = createSwapchainFramebuffers(context);
			INFO << "recreated swapchain!\n";
		}
	}

	void startFrame(VulkanAppContext* context) {

		//the current frameindex, is not the same as the image index as it is possible to have more images in the swapchain than frames being worked on
		uint32 frameindex = context->frameindex;

		//this is the index of the image in the swapchain that we are currently going to render to. it is aquired with vkAcquireNextImageKHR
		uint32_t imageindex;
		vkAcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, context->imageAvailableSemaphores[frameindex], VK_NULL_HANDLE, &imageindex);

		context->imageindex = imageindex;

		//we have to wait if the rendering on this frame from the last time we submitted rendercommands to this frame is not yet done. 
		//after the wait is done we have to manually reset the fence
		vkWaitForFences(context->device, 1, &context->inFlightFences[frameindex], VK_TRUE, UINT64_MAX);
		vkResetFences(context->device, 1, &context->inFlightFences[frameindex]);
		
		//resetting and recording the rendercommands again to the commandbuffer
		vkResetCommandBuffer(context->commandbuffers[frameindex], 0);	

		//begin a command buffer to record the drawcommands in
		VkCommandBufferBeginInfo begininfo{};
		begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begininfo.flags = 0; // Optional
		begininfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(context->commandbuffers[context->frameindex], &begininfo) != VK_SUCCESS) {
			ERR << "failed to begin recording command buffer!\n";
		}
	}

	void submitFrame(VulkanAppContext* context) {

		uint32 frameindex = context->frameindex;
		VkCommandBuffer currentcommandbuffer = context->commandbuffers[context->frameindex];

		if (vkEndCommandBuffer(currentcommandbuffer) != VK_SUCCESS) {
			ERR << "failed to end command buffer: " << frameindex << "\n";
		}

		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//the color attachmentoutput rendering commands that are submitted in this commandbuffer should only start when the image is available
		VkSemaphore waitsemaphores[] = { context->imageAvailableSemaphores[frameindex] };
		VkPipelineStageFlags waitstages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitinfo.waitSemaphoreCount = ARRAY_LENGTH(waitsemaphores);
		submitinfo.pWaitSemaphores = waitsemaphores;
		submitinfo.pWaitDstStageMask = waitstages;

		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &context->commandbuffers[frameindex];

		//signal the renderfinished semaphore once the commandbuffer is finished executing
		VkSemaphore signalsemaphores[] = { context->renderFinishedSemaphores[frameindex] };
		submitinfo.signalSemaphoreCount = ARRAY_LENGTH(signalsemaphores);
		submitinfo.pSignalSemaphores = signalsemaphores;

		if (vkQueueSubmit(context->graphicsqueue, 1, &submitinfo, context->inFlightFences[frameindex]) != VK_SUCCESS) {
			ERR << "failed to submit queue!\n";
		}

		//presenting the image to the swapchain;
		VkPresentInfoKHR presentinfo{};
		presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		//the frame can only be presented when the render finished semaphore is signaled.
		presentinfo.waitSemaphoreCount = ARRAY_LENGTH(signalsemaphores);
		presentinfo.pWaitSemaphores = signalsemaphores;

		VkSwapchainKHR swapchains[] = { context->swapchain };
		presentinfo.swapchainCount = 1;
		presentinfo.pSwapchains = swapchains;
		presentinfo.pImageIndices = &context->imageindex;

		presentinfo.pResults = nullptr;
		vkQueuePresentKHR(context->presentqueue, &presentinfo);

		context->frameindex = (context->frameindex + 1) % MAX_FRAMES_IN_FLIGHT;
	}


	void vulkanCleanup(VulkanAppContext* context) {

	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		INFO << "validation layer: " << pCallbackData->pMessage << "\n\n\n";

		return VK_FALSE;
	}
}
