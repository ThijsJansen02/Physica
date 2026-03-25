#pragma once
#include "Display.h"

namespace PH::Engine {

	//the global display renderpass that is shared between all displays. engine init initializes this.
	PH::Platform::GFX::RenderpassDescription Display::defaultrenderpassdescription;

	PH::Platform::GFX::RenderpassDescription createDisplayRenderpass() {

		Platform::GFX::RenderpassDescription renderpass;

		//create framebuffer for our purpose
		Platform::GFX::AttachmentDescription colorattachment;
		colorattachment.initiallayout = Platform::GFX::IMAGE_LAYOUT_UNDEFINED;
		colorattachment.finallayout = Platform::GFX::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		colorattachment.format = Platform::GFX::FORMAT_R8G8B8A8_UNORM;
		colorattachment.loadop = Platform::GFX::ATTACHMENT_LOAD_OP_CLEAR;
		colorattachment.storeop = Platform::GFX::ATTACHMENT_STORE_OP_STORE;

		Platform::GFX::AttachmentDescription depthattachment{};
		depthattachment.initiallayout = Platform::GFX::IMAGE_LAYOUT_UNDEFINED;
		depthattachment.finallayout = Platform::GFX::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthattachment.format = Platform::GFX::FORMAT_D32_SFLOAT;
		depthattachment.loadop = Platform::GFX::ATTACHMENT_LOAD_OP_CLEAR;
		depthattachment.storeop = Platform::GFX::ATTACHMENT_STORE_OP_DONT_CARE;

		Platform::GFX::AttachmentReference colorattachmentref{};
		colorattachmentref.attachmentindex = 0;
		colorattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference depthattachmentref{};
		depthattachmentref.attachmentindex = 1;
		depthattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference attachmentrefs[] = {
			colorattachmentref, depthattachmentref
		};

		Platform::GFX::SubPass subpass{};
		subpass.bindpoint = Platform::GFX::PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorattachments = { &colorattachmentref, 1 };
		subpass.depthstencilattachment = &depthattachmentref;

		/*
		Platform::GFX::SubpassDependency dependency{};
		dependency.srcsubpass = 0;
		dependency.dstsubpass = 0;
		dependency.dststagemask = PH::Platform::GFX::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcstagemask = PH::Platform::GFX::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstaccessmask = PH::Platform::GFX::ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		*/

		Platform::GFX::AttachmentDescription attachmentdescriptions[] = { colorattachment, depthattachment };

		Platform::GFX::RenderpassDescriptionCreateinfo renderpasscreate{};
		renderpasscreate.attachments = { attachmentdescriptions, ARRAY_LENGTH(attachmentdescriptions) };
		renderpasscreate.subpasses = { &subpass, 1 };
		renderpasscreate.dependencies = { nullptr, 0 };

		Platform::GFX::createRenderpassDescriptions(&renderpasscreate, &renderpass, 1);

		return renderpass;
	}

	//might move to seperate dislpay translation unit
	Display createDisplay(uint32 width, uint32 height) {

		Display display;
		display.renderpass = Display::defaultrenderpassdescription;

		Platform::GFX::TextureCreateInfo texturecreate{};
		texturecreate.format = Platform::GFX::FORMAT_R8G8B8A8_UNORM;
		texturecreate.width = width;
		texturecreate.height = height;
		texturecreate.data = nullptr;
		texturecreate.usage = Platform::GFX::IMAGE_USAGE_SAMPLED_BIT | Platform::GFX::IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		Platform::GFX::TextureCreateInfo depthcreate{};
		depthcreate.format = Platform::GFX::FORMAT_D32_SFLOAT;
		depthcreate.width = width;
		depthcreate.height = height;
		depthcreate.data = nullptr;
		depthcreate.usage = Platform::GFX::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		PH::Platform::GFX::Texture attachments[2];
		PH::Platform::GFX::TextureCreateInfo createinfos[] = { texturecreate, depthcreate };
		Platform::GFX::createTextures(createinfos, attachments, 2);

		display.colorattachment = attachments[0];
		display.depthattachment = attachments[1];

		Platform::GFX::FramebufferCreateInfo fbcreate;
		fbcreate.attachments = { attachments, ARRAY_LENGTH(attachments)};
		fbcreate.width = width;
		fbcreate.height = height;
		fbcreate.renderpassdescription = display.renderpass;

		Platform::GFX::createFramebuffers(&fbcreate, &display.fb, 1);

		display.framebuffersize = { width, height };
		display.viewport = { width, height };

		return display;
	}

	bool32 beginRenderPass(const Display& display) {

		//begin the scene render pass
		Platform::GFX::RenderpassbeginInfo renderpassbegin{};
		renderpassbegin.description = display.renderpass;
		renderpassbegin.framebuffer = display.fb;
		renderpassbegin.renderarea = { display.framebuffersize.x, display.framebuffersize.y };

		Platform::GFX::ClearValue clearvalues[2];
		clearvalues[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearvalues[1] = { 1.0f, 0 };

		renderpassbegin.clearvalues = { clearvalues, ARRAY_LENGTH(clearvalues) };

		Platform::GFX::beginRenderpass(&renderpassbegin);

		Platform::GFX::Viewport viewport;
		viewport.width = (real32)display.viewport.x;
		viewport.height = (real32)display.viewport.y;
		viewport.x = 0;
		viewport.y = 0;

		Platform::GFX::setViewports(&viewport, 1);

		Platform::GFX::Scissor scissor;
		scissor.width = (uint32)display.viewport.x;
		scissor.height = (uint32)display.viewport.y;
		scissor.x = 0;
		scissor.y = 0;
		Platform::GFX::setScissors(&scissor, 1);

		return true;
	}
	bool32 endRenderPass(const Display& display) {
		Platform::GFX::endRenderpass();

		return true;
	}

	ImGuiDisplay createImGuiDisplay(uint32 width, uint32 height) { 

		ImGuiDisplay result{};
		static_cast<Display&>(result) = createDisplay(width, height);
		result.imguitexture = Platform::GFX::createImGuiImage(result.colorattachment);
		
		return result;
	}

}