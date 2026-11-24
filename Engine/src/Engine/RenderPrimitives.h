#pragma once
#include <Base/Base.h>
#include <glm/glm.hpp>

namespace PH::Engine {



	struct Box2D {
		static Box2D create(real32 l, real32 r, real32 t, real32 b) {
			Box2D box;
			box.left = l;
			box.right = r;
			box.top = t;
			box.bottom = b;
			return box;
		}
		union {
			struct {
				glm::vec2 bottomleft;
				glm::vec2 topright;
			};

			struct {
				real32 left, bottom;
				real32 right, top;
			};

		};
	};

	inline Box2D operator+(const Box2D& b, glm::vec2 p) {
		return Box2D::create(b.left + p.x, b.right + p.x, b.top + p.y, b.bottom + p.y);
	}

	//returns wheter a point is in the give box;
	inline bool32 isInBox2D(const Box2D& box, glm::vec2 point) {
		return point.x > box.left && point.x < box.right && point.y < box.top && point.y > box.bottom;
	}

}