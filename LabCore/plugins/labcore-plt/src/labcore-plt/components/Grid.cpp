#include "Grid.h"
#include <LabCore/LabCoreAPI.h>

namespace PH::LabCorePLT {

	static bool32 serialize(YAML::Emitter& out, entt::entity entity, LabCore::Scene* scene) {
		out << YAML::Value << "No data";
		return true;
	}

	static bool32 deserialize(YAML::Node in, entt::entity entity, LabCore::Scene* scene) {
		return true;
	}

	static bool32 drawUI(entt::entity entity, LabCore::Scene* scene) {
		return true;
	}

	int64 calcOrder(real64 value) {
		return Base::floorReal64(log10(value));
	}

	bool32 draw(LabCore::Scene* scene) {
		Engine::BatchRenderer2D* renderer = LabCore::getRenderer2D();

		if (!scene->transforms.orthographic) {
			return false;
		}

		glm::mat4 inverseviewproj = glm::inverse(scene->transforms.projection * scene->transforms.view);

		glm::vec2 scale = 2.0f * glm::mat2(inverseviewproj) * glm::vec2(1.0f);

		real64 xgranularity = Base::pow_b_Uint64_exp_Int64(10, calcOrder(scale.x) - 1);
		real64 ygranularity = Base::pow_b_Uint64_exp_Int64(10, calcOrder(scale.y) - 1);
		
		//could probably make a matrix out of this but is only called once per frame so not performance critical
		real64 left = (inverseviewproj * glm::vec4(-1.0f, 0.0, 0.0, 1.0f)).x;
		real64 right = (inverseviewproj * glm::vec4(1.0f, 0.0, 0.0, 1.0f)).x;
		real64 top = (inverseviewproj * glm::vec4(0.0f, 1.0, 0.0, 1.0f)).y;
		real64 bottom = (inverseviewproj * glm::vec4(0.0f, -1.0, 0.0, 1.0f)).y;

		real64 xstart = Base::floorReal64(left / xgranularity) * xgranularity;
		real64 ystart = Base::floorReal64(bottom / ygranularity) * ygranularity;

		for (auto ent : scene->registry.view<Grid2D>()) {
			
			while (ystart < top) {
				renderer->drawLine({ left, ystart }, { right, ystart }, 0.0f, 1.0f, glm::mat2(scene->transforms.worldtoscreen), glm::vec4(0.2f));
				ystart += ygranularity;
			}

			while (xstart < right) {
				renderer->drawLine({ xstart, bottom }, { xstart, top }, 0.0f, 1.0f, glm::mat2(scene->transforms.worldtoscreen), glm::vec4(0.2f));
				xstart += xgranularity;
			}

			renderer->drawLine({ left, 0.0f }, { right, 0.0f }, -0.5f, 1.0f, glm::mat2(scene->transforms.worldtoscreen), glm::vec4{ 0.5f, 0.0f, 0.0f, 1.0f });
			renderer->drawLine({ 0.0f, bottom }, { 0.0f, top}, -0.5f, 1.0f, glm::mat2(scene->transforms.worldtoscreen), glm::vec4{0.0f, 0.5f, 0.0f, 1.0f});
		}
		return true;
	}

	static bool32 hasComponent(entt::entity entity, LabCore::Scene* scene) {
		return scene->registry.any_of<Grid2D>(entity);
	}

	static bool32 addComponent(entt::entity entity, LabCore::Scene* scene) {
		scene->registry.emplace<Grid2D>(entity, Grid2D{});
		return true;
	}

	LabCore::SceneComponentDescription Grid2D::createDescription() {
		LabCore::SceneComponentDescription desc{};
		desc.hasComponent_ = hasComponent;
		desc.deserializeComponent_ = deserialize;
		desc.serializeComponent_ = serialize;
		desc.addComponent = addComponent;
		desc.drawUI_ = drawUI;
		desc.drawComponents_ = draw;
		desc.name = Engine::String::create(getComponentName());
		return desc;
	}


}