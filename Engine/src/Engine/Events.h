#include "Engine.h"

namespace PH::Engine::Events {

	PH_ENGINE_API bool32 isKeyPressed(uint32 keycode);
   	PH_ENGINE_API bool32 isMouseButtonPressed(uint32 mousebuttoncode);

	void onEvent(PH::Platform::Event e);
	void startNewFrame();

	PH_ENGINE_API glm::vec2 getMousePos();
	PH_ENGINE_API glm::vec2 getDeltaMousePos();
}