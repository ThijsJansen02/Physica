#pragma once
#include "Engine.h"
#include "scene.h"
#include "assets/Mesh.h"
#include "assets/Material.h"
#include "assets/Texture.h"

namespace PH::Engine {

	struct TransformComponent {
		glm::mat4 local;
		glm::mat4 global;
	};

	struct QuadComponent {
		glm::vec4 color;

		UUID textureid;
		Assets::TextureImage* texture;
	};

	struct MeshComponent {

		UUID meshid;
		Engine::Assets::Mesh* mesh;

		Engine::DynamicArray<Assets::MaterialInstance*> materials;
	};

	struct TagComponent {
		uint64 uuid;
		String tag;
	};

	enum struct LightType {
		POINT = 0,
		DIRECTIONAL
	};

	struct LightComponent {
		glm::vec3 tint;
		real32 brightness;
		real32 linear;
		real32 quadratic;

		LightType type;
	};

	struct CameraComponent {
		glm::mat4 projection;

		bool32 orthographic;

		union {
			struct {
				real32 FOV;
				real32 AspectRatio;
				real32 NearClip;
				real32 FarClip;
			};
			struct {
				real32 zoom;
				real32 farclip;
				real32 nearclip;
			};
		};
	};
}