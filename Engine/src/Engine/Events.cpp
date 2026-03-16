#include "Events.h"

namespace PH::Engine::Events {

#define MAX_KEY_CODE 0xE2
#define MAX_MOUSE_BUTTON_CODE 4

	uint32 keyboard[MAX_KEY_CODE];
	uint32 mousebuttons[MAX_MOUSE_BUTTON_CODE];

	glm::vec2 lastframemousepos = { 0.0f, 0.0f };
	glm::vec2 currentframemousepos = { 0.0f, 0.0f };

	void startNewFrame() {
		lastframemousepos = currentframemousepos;
	}

	glm::vec2 getMousePos() {
		return currentframemousepos;
	}

	glm::vec2 getDeltaMousePos() {
		return currentframemousepos - lastframemousepos;
	}
	
	bool32 isKeyPressed(uint32 keycode) {
		return keyboard[keycode] != 0;
	}

	bool32 isMouseButtonPressed(uint32 mousebuttoncode) {
		return mousebuttons[mousebuttoncode] != 0;
	}

	void onEvent(PH::Platform::Event e) {
		if (e.type == PH_EVENT_TYPE_KEY_PRESSED) {
			keyboard[e.lparam] += 1;
		}
		if (e.type == PH_EVENT_TYPE_KEY_RELEASED) {
			keyboard[e.lparam] = 0;
		}
		if (e.type == PH_EVENT_TYPE_MOUSE_MOVED) {
			currentframemousepos = glm::vec2((real32)e.lparam, (real32)e.rparam);
		}
		if (e.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
			mousebuttons[e.lparam] = 1;
		}
		if (e.type == PH_EVENT_TYPE_MOUSEBUTTON_RELEASED) {
			mousebuttons[e.lparam] = 0;
		}
	}

}