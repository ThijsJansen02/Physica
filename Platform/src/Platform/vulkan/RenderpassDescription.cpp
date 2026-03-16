#include "Platform/vulkan/vulkanAPI.h"

namespace PH::Vulkan {

	bool32 createRenderpassDescription(
		VulkanAppContext* context, 
		PH::Platform::GFX::RenderpassDescriptionCreateinfo* createinfos, 
		PH::Platform::GFX::RenderpassDescription* descriptions, 
		PH::uint32 count
	)  {

		//get arena resetpoint
		PH::Base::MemoryArenaReset resetpoint = ArenaAllocator::getResetPoint();

		for (uint32 i = 0; i < count; i++) {
			PH::Platform::GFX::RenderpassDescriptionCreateinfo& createinfo = createinfos[i];

			auto attachments = Base::ArrayList<VkAttachmentDescription, ArenaAllocator>::create(createinfo.attachments.count);
			for (const Platform::GFX::AttachmentDescription& des : createinfo.attachments) {

				VkAttachmentDescription attachment{};
				attachment.format = convertFormat(des.format);
				attachment.samples = VK_SAMPLE_COUNT_1_BIT;

				attachment.loadOp = convertAttachmentLoadOp(des.loadop);
				attachment.storeOp = convertAttachmentStoreOp(des.storeop);

				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				attachment.initialLayout = convertImageLayout(des.initiallayout);

				attachment.finalLayout = convertImageLayout(des.finallayout);

				attachments.pushBack(attachment);
			}

			auto subpasses = Base::ArrayList<VkSubpassDescription, ArenaAllocator>::create(createinfo.subpasses.count);
			for (const Platform::GFX::SubPass& subpass : createinfo.subpasses) {
				
				auto colorattachmentreferences = Base::ArrayList<VkAttachmentReference, ArenaAllocator>::create(subpass.colorattachments.count);
				for (const Platform::GFX::AttachmentReference& ref : subpass.colorattachments) {

					VkAttachmentReference vkref{};
					vkref.attachment = ref.attachmentindex;
					vkref.layout = convertImageLayout(ref.layout);
					colorattachmentreferences.pushBack(vkref);
				}

				VkSubpassDescription description{};
				description.colorAttachmentCount = (uint32)colorattachmentreferences.getCount();
				description.pColorAttachments = colorattachmentreferences.raw();

				VkAttachmentReference depthstencil{};
				if (subpass.depthstencilattachment) {
					depthstencil.attachment = subpass.depthstencilattachment->attachmentindex;
					depthstencil.layout = convertImageLayout(subpass.depthstencilattachment->layout);
					description.pDepthStencilAttachment = &depthstencil;
				}
				
				description.pipelineBindPoint = convertPipelineBindPoint(subpass.bindpoint);
				subpasses.pushBack(description);
			}

			auto dependencies = Base::ArrayList<VkSubpassDependency, ArenaAllocator>::create(createinfo.dependencies.count);
			for (const Platform::GFX::SubpassDependency& dep : createinfo.dependencies) {
					
				VkSubpassDependency vkdep{};
				vkdep.srcSubpass = dep.srcsubpass;
				vkdep.dstSubpass = dep.dstsubpass;

				vkdep.dstAccessMask = convertAccessFlags(dep.dstaccessmask);
				vkdep.dstStageMask = convertPipelineStageFlags(dep.dststagemask);

				vkdep.srcAccessMask = convertAccessFlags(dep.srcaccessmask);
				vkdep.srcStageMask = convertPipelineStageFlags(dep.srcstagemask);

				dependencies.pushBack(vkdep);
			}

			VkRenderPassCreateInfo vkcreateinfo{};

			vkcreateinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			vkcreateinfo.attachmentCount = (uint32)attachments.getCount();
			vkcreateinfo.dependencyCount = (uint32)dependencies.getCount();
			vkcreateinfo.subpassCount = (uint32)subpasses.getCount();
			vkcreateinfo.pAttachments = attachments.raw();
			vkcreateinfo.pDependencies = dependencies.raw();
			vkcreateinfo.pSubpasses = subpasses.raw();

			VkRenderPass vkrenderpass;
			if (vkCreateRenderPass(context->device, &vkcreateinfo, nullptr, &vkrenderpass) != VK_SUCCESS) {
				ERR << "failed to create renderpass!\n";
				ArenaAllocator::resetArena(resetpoint);
				return false;
			}

			Renderpass renderpass;
			renderpass.renderpass = vkrenderpass;

			descriptions[i] = (PH::Platform::GFX::RenderpassDescription)context->renderpasses.add(renderpass);
		}
		ArenaAllocator::resetArena(resetpoint);

		return true;
	}

	PH_GFX_CREATE_RENDERPASS_DESCRIPTIONS(createRenderpassDescription) {
		return createRenderpassDescription(context_s, createinfos, descriptions, count);
	}

	bool32 beginRenderpass(VulkanAppContext* context, PH::Platform::GFX::RenderpassbeginInfo* begininfo) {

		VkCommandBuffer buffer = context->commandbuffers[context->frameindex];
		VkFramebuffer vkframebuffer{};

		//if the framebuffer id == 0, then well use the swapchain framebuffers
		if (begininfo->framebuffer == 0) {
			vkframebuffer = context->swapchainframebuffers[context->imageindex];
		}
		else {
			vkframebuffer = context->framebuffers[begininfo->framebuffer].buffer;
		}

		VkRenderPass vkrenderpass = context->renderpasses[begininfo->description].renderpass;

		VkRenderPassBeginInfo vkbegininfo{};
		vkbegininfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkbegininfo.framebuffer = vkframebuffer;

		vkbegininfo.clearValueCount = begininfo->clearvalues.count;
		vkbegininfo.pClearValues = (VkClearValue*)begininfo->clearvalues.data;

		vkbegininfo.renderArea.offset = { 0, 0 };
		vkbegininfo.renderArea.extent.width = begininfo->renderarea.width;
		vkbegininfo.renderArea.extent.height = begininfo->renderarea.height;
		vkbegininfo.renderPass = vkrenderpass;

		vkCmdBeginRenderPass(buffer, &vkbegininfo, VK_SUBPASS_CONTENTS_INLINE);
		return true;
	}

	PH_GFX_BEGIN_RENDERPASS(beginRenderpass) {
		return beginRenderpass(context_s, renderpassbegininfo);
	}

	bool32 endRenderpass(VulkanAppContext* context) {
		VkCommandBuffer vkbuffer = context->commandbuffers[context->frameindex];
		vkCmdEndRenderPass(vkbuffer);
		return true;
	}

	PH_GFX_END_RENDERPASS(endRenderpass) {
		return endRenderpass(context_s);
	}


	bool32 createFramebuffers(
		VulkanAppContext* context,
		PH::Platform::GFX::FramebufferCreateInfo* createinfos,
		PH::Platform::GFX::Framebuffer* framebuffers,
		PH::uint32 count) {


		PH::Base::MemoryArenaReset resetpoint = ArenaAllocator::getResetPoint();
		
		for (uint32 i = 0; i < count; i++) {

			const auto& createinfo = createinfos[i];

			auto attachments = PH::Base::ArrayList<VkImageView, ArenaAllocator>::create(createinfo.attachments.count);
			
			for (auto texture : createinfo.attachments) {
				auto textureview = context->textureimages[texture].view;
				attachments.pushBack(textureview);
			}

			VkFramebufferCreateInfo framebuffercreate{};
			framebuffercreate.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffercreate.attachmentCount = attachments.getCount();
			framebuffercreate.pAttachments = attachments.raw();

			framebuffercreate.height = createinfo.height;
			framebuffercreate.width = createinfo.width;
			framebuffercreate.renderPass = context->renderpasses[createinfo.renderpassdescription].renderpass;
			framebuffercreate.layers = 1;

			Framebuffer result;
			if (vkCreateFramebuffer(context->device, &framebuffercreate, nullptr, &result.buffer) != VK_SUCCESS) {
				WARN << "failed to create frame buffer";
				PH_DEBUG_BREAK();
				framebuffers[i] = 0;
				return false;
			}
			framebuffers[i] = context->framebuffers.add(result);
		}

		return true;
	}

	//might want to move this to a seperate framebuffer cpp file
	PH_GFX_CREATE_FRAMEBUFFERS(createFramebuffers) {
		return createFramebuffers(context_s, createinfos, framebuffers, count);
	}

}

