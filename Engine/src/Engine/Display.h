#pragma once
#include "Engine.h"

namespace PH::Engine {

	/// <summary>
	/// holds the framebuffer and colorattachment for a dispay... 
	/// might make it more dynamice in the future to hold multiple colorattachments and depthattachment
	/// </summary>
	struct Display {

		PH::Platform::GFX::Framebuffer fb;
		PH::Platform::GFX::Texture colorattachment;
		PH::Platform::GFX::Texture depthattachment;

		glm::vec4 clearcolor;

		PH::Platform::GFX::RenderpassDescription renderpass;

		glm::uvec2 framebuffersize;
		glm::vec2 viewport;

		//the static display renderpass
		static PH::Platform::GFX::RenderpassDescription defaultrenderpassdescription;
	};

	inline real32 getDisplayAspectRatio(const Display& display) {
		return display.viewport.x / display.viewport.y;
	}

	bool32 PH_ENGINE_API beginRenderPass(const Display& display);
	bool32 PH_ENGINE_API endRenderPass(const Display& display);

	struct ImGuiDisplay : public Display {
		ImTextureID imguitexture;
	};

	Display PH_ENGINE_API createDisplay(uint32 width, uint32 height);
	ImGuiDisplay PH_ENGINE_API createImGuiDisplay(uint32 width, uint32 height);



}