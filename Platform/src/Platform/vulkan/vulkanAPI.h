#pragma once
#include "Platform/platformAPI.h"
#include "vulkan.h"

namespace PH::Vulkan {
	
	extern VulkanAppContext* context_s;
		
	inline VkFormat convertFormat(Platform::GFX::Format format) {
		return (VkFormat)format;
	};

	inline VkAttachmentLoadOp convertAttachmentLoadOp(Platform::GFX::AttachmentLoadOp loadop) {
		return (VkAttachmentLoadOp)loadop;
	}

	inline VkAttachmentStoreOp convertAttachmentStoreOp(Platform::GFX::AttachmentStoreOp storeop) {
		return (VkAttachmentStoreOp)storeop;
	}

	inline VkPipelineBindPoint convertPipelineBindPoint(Platform::GFX::PipelineBindPoint bindpoint) {
		return (VkPipelineBindPoint)bindpoint;
	}

	inline VkImageLayout convertImageLayout(Platform::GFX::ImageLayout layout) {
		return (VkImageLayout)layout;
	}
	
	inline VkAccessFlagBits convertAccessFlags(Platform::GFX::AccessFlags accessflags) {
		return (VkAccessFlagBits)accessflags;
	}

	inline VkPipelineStageFlags convertPipelineStageFlags(Platform::GFX::PipelineStageFlags stageflags) {
		return (VkPipelineStageFlags)stageflags;
	}

	inline VkShaderStageFlags convertShaderStageFlags(Platform::GFX::ShaderStageFlags stageflags) {
		return (VkPipelineStageFlags)stageflags;
	}
	
	inline VkPrimitiveTopology convertPrimitiveTopology(Platform::GFX::PrimitiveTopology topology) {
		return (VkPrimitiveTopology)topology;
	}

	inline VkBufferUsageFlags convertBufferUsage(Platform::GFX::BufferUsageFlags bufferusage) {
		return VkBufferUsageFlags(bufferusage);
	}

	inline VkVertexInputRate convertVertexInputrate(Platform::GFX::VertexInputRate inputrate) {
		return inputrate == Platform::GFX::INPUT_RATE_PER_VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
	}

	inline VkMemoryPropertyFlags convertMemoryPropertyflags(Platform::GFX::MemoryPropertyFlags flags) {
		return VkMemoryPropertyFlags(flags);
	}


	inline VkDescriptorType convertDescriptorType(Platform::GFX::DescriptorType flags) {
		return VkDescriptorType(flags);
	}

	inline VkImageUsageFlags convertImageUsage(Platform::GFX::ImageUsageFlags flags) {
		return VkImageUsageFlags(flags);
	}

	inline VkImageViewType convertImageViewType(Platform::GFX::ImageViewType viewtype) {
		return VkImageViewType(viewtype);
	}

	PH_GFX_DRAW(draw);
	PH_GFX_SET_VIEWPORTS(setViewports);
	PH_GFX_SET_SCISSORS(setScissors);
	PH_GFX_DRAW(drawIndexed);

	PH_GFX_CREATE_RENDERPASS_DESCRIPTIONS(createRenderpassDescription);
	PH_GFX_BEGIN_RENDERPASS(beginRenderpass);
	PH_GFX_END_RENDERPASS(endRenderpass);
	PH_GFX_CREATE_FRAMEBUFFERS(createFramebuffers);

	PH_GFX_CREATE_BUFFERS(createBuffers);
	PH_GFX_DESTROY_BUFFERS(destroyBuffers);
	PH_GFX_BIND_VERTEXBUFFERS(bindVertexBuffers);
	PH_GFX_BIND_INDEXBUFFER(bindIndexBuffer);
	PH_GFX_COPY_BUFFER(copyBuffer);
	PH_GFX_MAP_BUFFER(mapBuffer);
	PH_GFX_UNMAP_BUFFER(unmapBuffer);

	PH_GFX_CREATE_DESCRIPTOR_SET_LAYOUT(createDescriptorSetLayouts);
	PH_GFX_CREATE_DESCRIPTOR_SETS(createDescriptorSets);
	PH_GFX_DESTROY_DESCIPTOR_SET_LAYOUTS(destroyDescriptorSetLayouts);
	PH_GFX_DESTROY_DESCRIPTOR_SETS(destroyDescriptorSets);
	PH_GFX_UPDATE_DESCRIPTOR_SETS(updateDescriptorSets);
	PH_GFX_BIND_DESCRIPTOR_SETS(bindDescriptorSets);

	PH_GFX_CREATE_TEXTURES(createTextures);

	PH_GFX_CREATE_SHADERS(createShaders);
	PH_GFX_DESTROY_SHADERS(destroyShaders);
	PH_GFX_CREATE_GRAPHICS_PIPELINES(createGraphicsPipelines);
	PH_GFX_BIND_GRAPHICS_PIPELINE(bindGraphicsPipeline);

	void loadAPI(PH::Platform::Intern::PlatformAPI* api);
}
