#include "vulkanAPI.h"

namespace PH::Vulkan {

	//defined in Buffers.cpp;
	uint32 findMemoryType(VkPhysicalDevice physicaldevice, uint32 typefilter, VkMemoryPropertyFlags properties);

	void transitionImageLayout(VulkanAppContext* context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32 layercount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layercount;
		barrier.subresourceRange.levelCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			WARN << "unsupported layout transition!\n";
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(context, commandBuffer);
	}

	void copyBufferToImage(VulkanAppContext* context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32 layercount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layercount;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(context, commandBuffer);
	}

	/// <summary>
	/// creates the imageview for the standard textureimage
	/// </summary>
	/// <param name="context">the context used in for this operation</param>
	/// <param name="image">the image to attach the view to</param>
	/// <param name="createinfo">the createinfo for the actual image</param>
	/// <returns>the image view that is created</returns>
	VkImageView createImageView(VulkanAppContext* context, VkImage image, const PH::Platform::GFX::TextureCreateInfo& createinfo) {
		
		VkImageViewCreateInfo viewinfo{};
		viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewinfo.image = image;
		viewinfo.viewType = convertImageViewType(createinfo.viewtype);
		viewinfo.format = convertFormat(createinfo.format);

		viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		if (createinfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		viewinfo.subresourceRange.baseMipLevel = 0;
		viewinfo.subresourceRange.levelCount = 1;
		viewinfo.subresourceRange.baseArrayLayer = 0;
		viewinfo.subresourceRange.layerCount = createinfo.ArrayLayers;

		VkImageView view;

		if (vkCreateImageView(context->device, &viewinfo, nullptr, &view) != VK_SUCCESS) {
			WARN << "Failed to create image view\n";
			PH_DEBUG_BREAK();
		}
		return view;
	}

	VkSampler createImageSampler(VulkanAppContext* context, const PH::Platform::GFX::TextureCreateInfo& createinfo) {

		VkSamplerCreateInfo samplerinfo{};
		samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerinfo.magFilter = VK_FILTER_LINEAR;
		samplerinfo.minFilter = VK_FILTER_LINEAR;

			
		samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerinfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(context->physicaldevice, &properties);

		samplerinfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerinfo.unnormalizedCoordinates = VK_FALSE;

		samplerinfo.compareEnable = VK_FALSE;
		samplerinfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerinfo.mipLodBias = 0.0f;
		samplerinfo.minLod = 0.0f;
		samplerinfo.maxLod = 0.0f;

		VkSampler sampler;

		if (vkCreateSampler(context->device, &samplerinfo, nullptr, &sampler) != VK_SUCCESS) {
			WARN << "Failed to create samplers\n";
			PH_DEBUG_BREAK();
		}

		return sampler;
	}

	PH::bool32 destroyTexture(VulkanAppContext* context, PH::Platform::GFX::Texture* textures, PH::uint32 count) {
		for (uint32 i = 0; i < count; i++) {
			auto& texture = textures[i];
			auto& textureimage = context->textureimages[texture];
			vkDestroyImageView(context->device, textureimage.view, nullptr);
			vkDestroyImage(context->device, textureimage.image, nullptr);
			vkFreeMemory(context->device, textureimage.imagememory, nullptr);
			vkDestroySampler(context->device, textureimage.sampler, nullptr);
		}

		return true;
	}

	PH::bool32 createTextures(
		VulkanAppContext* context, 
		PH::Platform::GFX::TextureCreateInfo* createinfos, 
		PH::Platform::GFX::Texture* textures,
		uint32 count) 
	
	{
		for (uint32 i = 0; i < count; i++) {

			Vulkan::TextureImage textureimage;

			auto& createinfo = createinfos[i];

			VkImageCreateInfo imagecreateinfo{};
			imagecreateinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imagecreateinfo.imageType = VK_IMAGE_TYPE_2D;

			//sets the width and height of the image;
			imagecreateinfo.extent.width = createinfo.width;
			imagecreateinfo.extent.height = createinfo.height;
			imagecreateinfo.extent.depth = 1;

			//sets the amount of mipmaps the image gets
			imagecreateinfo.mipLevels = 1;
			imagecreateinfo.arrayLayers = createinfo.ArrayLayers;

			//the tiling of the image changes the way the texels are layed out in memory
			imagecreateinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imagecreateinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			//usage is sampled image and transfer destination because we are going to copy the texel data from a staging buffer
			imagecreateinfo.usage = convertImageUsage(createinfo.usage);
			if (createinfo.data) {
				imagecreateinfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}

			imagecreateinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			//the format of the image
			imagecreateinfo.format = convertFormat(createinfo.format);

			
			//the amout of samples taken from the image, is related to multisampling
			imagecreateinfo.samples = VK_SAMPLE_COUNT_1_BIT;

			imagecreateinfo.flags = 0;
			if (createinfo.viewtype == Platform::GFX::IMAGE_VIEW_TYPE_CUBE) {
				imagecreateinfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			}

			if (vkCreateImage(context->device, &imagecreateinfo, nullptr, &textureimage.image) != VK_SUCCESS) {
				WARN << "failed to create texture image\n";
				PH_DEBUG_BREAK();
				return false;
			}

			//now the device memory is created with vkallocatememory
			VkMemoryRequirements memrequirements{};
			vkGetImageMemoryRequirements(context->device, textureimage.image, &memrequirements);

			VkMemoryAllocateInfo allocinfo{};
			allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocinfo.memoryTypeIndex = findMemoryType(context->physicaldevice, memrequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			allocinfo.allocationSize = memrequirements.size;

			if (vkAllocateMemory(context->device, &allocinfo, nullptr, &textureimage.imagememory) != VK_SUCCESS) {
				WARN << "failed to create texture memory!\n";
				PH_DEBUG_BREAK();
				return false;
			}

			vkBindImageMemory(context->device, textureimage.image, textureimage.imagememory, 0);

			if (createinfo.data) {

				//transition image to a layout optimal for copying data to
				transitionImageLayout(
					context, 
					textureimage.image, 
					convertFormat(createinfo.format),
					VK_IMAGE_LAYOUT_UNDEFINED, //initial layout
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //final layout
					createinfo.ArrayLayers
				);

				//creating a staging buffer to set the memory up to transfer to the actual image.
				PH::Platform::GFX::BufferCreateinfo buffercreate{};
				buffercreate.bufferusage = Platform::GFX::BUFFER_USAGE_TRANSFER_SRC_BIT;
				buffercreate.data = createinfo.data;

				//TODO(Thijs): make the size depend on the format
				buffercreate.size = createinfo.width * createinfo.height * 4 * createinfo.ArrayLayers;
				buffercreate.dynamic = false;
				buffercreate.memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;
				
				PH::Platform::GFX::Buffer buffer;
				createBuffers(&buffercreate, &buffer, 1);

				VkBuffer stagingbuffer = context->buffers[buffer].buffer[0];

				copyBufferToImage(context, stagingbuffer, textureimage.image, static_cast<uint32_t>(createinfo.width), static_cast<uint32_t>(createinfo.height), createinfo.ArrayLayers);

				//transitioning image to a layout optimal for sampling with a shader.
				transitionImageLayout(
					context,
					textureimage.image,
					convertFormat(createinfo.format),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					createinfo.ArrayLayers
				);

				destroyBuffers(&buffer, 1);
			}
			else {
				if (createinfo.usage & PH::Platform::GFX::IMAGE_USAGE_SAMPLED_BIT) {
					transitionImageLayout(
						context,
						textureimage.image,
						convertFormat(createinfo.format),
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						createinfo.ArrayLayers
					);
				}

			}

			textureimage.view = createImageView(context, textureimage.image, createinfo);
			textureimage.sampler = createImageSampler(context, createinfo);

			textures[i] = context->textureimages.add(textureimage);
		}
		return true;
	}

	PH_GFX_DESTROY_TEXTURES(destroyTextures) {
		return destroyTexture(context_s, textures, count);
	}

	PH_GFX_CREATE_TEXTURES(createTextures) {
		return createTextures(context_s, createinfos, textures, count);
	}



}
