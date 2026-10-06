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

		colorattachment.format = Platform::GFX::FORMAT_R8G8B8A8_SRGB;
		colorattachment.loadop = Platform::GFX::ATTACHMENT_LOAD_OP_CLEAR;
		colorattachment.storeop = Platform::GFX::ATTACHMENT_STORE_OP_STORE;

		Platform::GFX::AttachmentReference colorattachmentref{};
		colorattachmentref.attachmentindex = 0;
		colorattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		Platform::GFX::AttachmentReference attachmentrefs[] = {
			colorattachmentref
		};

		Platform::GFX::SubPass subpass{};
		subpass.bindpoint = Platform::GFX::PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorattachments = { &colorattachmentref, 1 };
		subpass.depthstencilattachment = PH_GFX_NULL;

		
		Platform::GFX::SubpassDependency dependency{};
		dependency.srcsubpass = GFX_SUBPASS_EXTERNAL;
		dependency.dstsubpass = 0;
		dependency.dststagemask = PH::Platform::GFX::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcstagemask = PH::Platform::GFX::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstaccessmask = PH::Platform::GFX::ACCESS_MEMORY_WRITE_BIT;
		dependency.srcaccessmask = PH::Platform::GFX::ACCESS_MEMORY_WRITE_BIT;

		Platform::GFX::AttachmentDescription attachmentdescriptions[] = { colorattachment };

		Platform::GFX::RenderpassDescriptionCreateinfo renderpasscreate{};
		renderpasscreate.attachments = { attachmentdescriptions, ARRAY_LENGTH(attachmentdescriptions) };
		renderpasscreate.subpasses = { &subpass, 1 };
		renderpasscreate.dependencies = { &dependency, 1 };

		Platform::GFX::createRenderpassDescriptions(&renderpasscreate, &renderpass, 1);

		return renderpass;
	}

	Display createDisplay(uint32 width, uint32 height) {

		Display display;
		display.renderpass = Display::defaultrenderpassdescription;

		Platform::GFX::TextureCreateInfo texturecreate{};
		texturecreate.format = Platform::GFX::FORMAT_R8G8B8A8_SRGB;
		texturecreate.width = width;
		texturecreate.height = height;
		texturecreate.data = nullptr;
		texturecreate.usage = Platform::GFX::IMAGE_USAGE_SAMPLED_BIT | Platform::GFX::IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		PH::Platform::GFX::Texture attachments[1];
		PH::Platform::GFX::TextureCreateInfo createinfos[] = { texturecreate };
		Platform::GFX::createTextures(createinfos, attachments, 1);

		display.colorattachment = attachments[0];
		display.depthattachment = PH_GFX_NULL;

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