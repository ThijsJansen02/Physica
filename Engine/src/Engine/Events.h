#include "Engine.h"

namespace PH::Engine::Events {

	bool32 isKeyPressed(uint32 keycode);
	bool32 isMouseButtonPressed(uint32 mousebuttoncode);

	void onEvent(PH::Platform::Event e);
	void startNewFrame();

	glm::vec2 getMousePos();
	glm::vec2 getDeltaMousePos();
}