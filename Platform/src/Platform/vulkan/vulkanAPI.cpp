
#include "vulkanAPI.h"


namespace PH::Vulkan {
	
	VulkanAppContext* context_s = nullptr;

	
	PH_GFX_DRAW(draw) {
		vkCmdDraw(context_s->commandbuffers[context_s->frameindex], vertexcount, instancecount, firstvertex, firstinstance);
		return true;
	}

	PH_GFX_DRAW_INDEXED(drawIndexed) {
		vkCmdDrawIndexed(context_s->commandbuffers[context_s->frameindex], indexcount, instancecount, firstindex, 0, firstinstance);
		return true;
	}

	PH_GFX_SET_VIEWPORTS(setViewports) {

		Base::MemoryArenaReset reset = ArenaAllocator::getResetPoint();

		auto vkviewports = Base::ArrayList<VkViewport, ArenaAllocator>::create(count);
		for (uint32 i = 0; i < count; i++) {
			const PH::Platform::GFX::Viewport& viewport = viewports[i];

			VkViewport vkviewport{};
			vkviewport.x = viewport.x;
			vkviewport.y = viewport.y;
			vkviewport.width = viewport.width;
			vkviewport.height = viewport.height;
			vkviewport.maxDepth = 1.0f;
			vkviewport.minDepth = 0.0f;

			vkviewports.pushBack(vkviewport);
		}

		vkCmdSetViewport(context_s->commandbuffers[context_s->frameindex], 0, count, vkviewports.raw());
		ArenaAllocator::resetArena(reset);

		return true;
	}

	PH_GFX_SET_SCISSORS(setScissors) {
		Base::MemoryArenaReset reset = ArenaAllocator::getResetPoint();

		auto vkrects = Base::ArrayList<VkRect2D, ArenaAllocator>::create(count);
		for (uint32 i = 0; i < count; i++) {
			const PH::Platform::GFX::Scissor& scissor = scissors[i];

			VkRect2D vkrect{};
			vkrect.offset.x = scissor.x;
			vkrect.offset.y = scissor.y;
			vkrect.extent.width = scissor.width;
			vkrect.extent.height = scissor.height;

			vkrects.pushBack(vkrect);
		}

		vkCmdSetScissor(context_s->commandbuffers[context_s->frameindex], 0, count, vkrects.raw());
		ArenaAllocator::resetArena(reset);

		return true;
	}

	void loadAPI(PH::Platform::Intern::PlatformAPI* api) {

		api->consolewrite = win32_consoleWrite;

		api->gfxdraw = PH::Vulkan::draw;
		api->gfxdrawindexed = PH::Vulkan::drawIndexed;

		api->gfxcreaterenderpassdescription = PH::Vulkan::createRenderpassDescription;
		api->gfxcreateframebuffers = PH::Vulkan::createFramebuffers;

		api->gfxbeginrenderpass = PH::Vulkan::beginRenderpass;
		api->gfxendrenderpass = PH::Vulkan::endRenderpass;

		api->gfxcreateshaders = PH::Vulkan::createShaders;
		api->gfxdestroyshaders = PH::Vulkan::destroyShaders;
		api->gfxcreategraphicspipelines = PH::Vulkan::createGraphicsPipelines;
		api->gfxbindgraphicspipeline = PH::Vulkan::bindGraphicsPipeline;

		api->gfxcreatedescriptorsetlayouts = PH::Vulkan::createDescriptorSetLayouts;
		api->gfxdestroydescriptorsets = PH::Vulkan::destroyDescriptorSets;
		api->gfxdestroydescriptorsetlayouts = PH::Vulkan::destroyDescriptorSetLayouts;
		api->gfxcreatedescriptorsets = PH::Vulkan::createDescriptorSets;
		api->gfxupdatedescriptorsets = PH::Vulkan::updateDescriptorSets;
		api->gfxbinddescriptorsets = PH::Vulkan::bindDescriptorSets;

		api->gfxcreatetextures = PH::Vulkan::createTextures;

		api->gfxcreatebuffers = PH::Vulkan::createBuffers;
		api->gfxbindvertexbuffers = PH::Vulkan::bindVertexBuffers;
		api->gfxbindindexbuffer = PH::Vulkan::bindIndexBuffer;
		api->gfxcopybuffers = PH::Vulkan::copyBuffer;
		api->gfxmapbuffer = PH::Vulkan::mapBuffer;
		api->gfxunmapbuffer = PH::Vulkan::unmapBuffer;

		api->gfxsetscissors = PH::Vulkan::setScissors;
		api->gfxsetviewports = PH::Vulkan::setViewports;

		return;

	}
}