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

	//returns wheter a point is in the give box;
	inline bool32 isInBox2D(const Box2D& box, glm::vec2 point) {
		return point.x > box.left && point.x < box.right && point.y < box.top && point.y > box.bottom;
	}

}