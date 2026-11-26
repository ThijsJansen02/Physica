#pragma once
#include <glm/glm.hpp>
#include <Engine/RenderPrimitives.h>
#include <Platform/platformAPI.h>

namespace PH::LabCorePLT {

	struct PlotCamera {
		glm::vec2 position;
		Engine::Box2D domain;

		glm::mat4 proj;
		glm::mat4 view;
	};

	glm::mat4 getPlotCameraProjectionMatrix(const PlotCamera& cam);
	glm::mat4 getPlotCameraViewMatrix(const PlotCamera& cam);
	glm::mat2 getScreenToWorldMatrix(const PlotCamera& cam, const glm::vec2& screensize);
	glm::mat4 getWorldToScreenSpace(const PlotCamera& cam, const glm::vec2& screensize);

	PlotCamera createPlotCamera(Engine::Box2D domain, glm::vec2 position);

	void resolvePlotCameraEvent(const PH::Platform::Event& e, PlotCamera* cam);
	void updateCamera(PlotCamera* cam);
}