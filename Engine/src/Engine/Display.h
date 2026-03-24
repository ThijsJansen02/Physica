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

		PH::Platform::GFX::RenderpassDescription renderpass;

		glm::uvec2 framebuffersize;
		glm::vec2 viewport;

		//the static display renderpass
		static PH::Platform::GFX::RenderpassDescription defaultrenderpassdescription;
	};

	
	

	inline real32 getDisplayAspectRatio(const Display& display) {
		return display.viewport.x / display.viewport.y;
	}

	bool32 beginRenderPass(const Display& display);
	bool32 endRenderPass(const Display& display);

	//first inheritance in years haha/ still no virtual function tho :)
	struct ImGuiDisplay : public Display {
		ImTextureID imguitexture;
	};

	Display createDisplay(uint32 width, uint32 height);
	ImGuiDisplay createImGuiDisplay(uint32 width, uint32 height);



}