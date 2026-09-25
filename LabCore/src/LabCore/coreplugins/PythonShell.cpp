
#include "PythonShell.h"
#include <Engine/Display.h>
#include <LabCore/LabCore.h>
#include <Engine/coreassets/Font.h>

namespace PH::LabCore {

	struct pythonShellInstance {
		Engine::ImGuiDisplay display;

		Engine::Font* font;
	};

	bool32 pythonShellUpdate(void* instancedata) {
		return true;
	}

	bool32 pythonShellInstantiate(pythonShellInstance* shell) {
		shell->display = Engine::createImGuiDisplay(1920, 1080);
		shell->font = appcontext->assets.getAssetByReference<Engine::Font>("arial");



		return true;
	}

	bool32 pythonShellDraw(pythonShellInstance* shell) {
		
		//physica draw part
		Engine::beginRenderPass(shell->display);

		renderer2D->begin();

		//set the projection and view matrices for the renderer2D wrapper, this is going to be used to draw the plot and the background of the plot, this is going to be set to the size of the display, so that we can draw the plot and the background of the plot in the correct position and size
		glm::mat4 projection = glm::ortho(0.0f, (real32)shell->display.viewport.x, (real32)shell->display.viewport.y, 0.0f);
		renderer2D->pushProjection(projection);
		renderer2D->pushView(glm::mat4(1.0f));

		renderer2D->pushGraphicsPipeline(LabCore::appcontext->defaultfontpipeline2D, { &shell->font->cdata, 1 });
		renderer2D->pushTexture(shell->font->atlas);
		
		Engine::drawText(shell->font, "Python Shell", { 0.0f, 0.0f }, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, renderer2D->getContext());

		renderer2D->end();
		renderer2D->flush({nullptr, 0});

		Engine::endRenderPass(shell->display);
		
		//imgui draw part
		real32 titlebarheight = ImGui::GetFrameHeight();
		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		shell->display.viewport = { displaysize.x, displaysize.y };
		ImGui::Image(
			shell->display.imguitexture,
			displaysize,
			{ 0.0f, 0.0f }, //bottom left uv
			{ displaysize.x / shell->display.framebuffersize.x, displaysize.y / shell->display.framebuffersize.y } //top right UV
		);
	

		return true;
	}

	bool32 pythonShellDestroy(void* instancedata) {
		return true;
	}

	bool32 pythonShellOnEvent(void* instancedata, const PH::Platform::Event& event) {

		if (event.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
			Engine::INFO << "clicked in window!\n";
		}

		return true;
	}

	View getPythonShellView() {

		View v{};
		v.instancedatasize = sizeof(pythonShellInstance);
		v.destroy_ = pythonShellDestroy;
		v.draw_ = (ViewDrawFunction)pythonShellDraw;
		v.onEvent_ = pythonShellOnEvent;
		v.onUpdate_ = pythonShellUpdate;
		v.instantiate_ = (ViewInstantiateFunction)pythonShellInstantiate;
		v.name = Engine::String::create("Python Shell");

		return v;
	}
}