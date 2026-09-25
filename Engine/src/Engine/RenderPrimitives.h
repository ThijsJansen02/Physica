#pragma once
#include <Base/Base.h>
#include <glm/glm.hpp>

namespace PH::Engine {

	struct Box2D {
		
		union {
			struct {
				real32 left, bottom;
				real32 right, top;
			};

			struct {
				glm::vec2 bottomleft;
				glm::vec2 topright;
			};
		};

	};

}