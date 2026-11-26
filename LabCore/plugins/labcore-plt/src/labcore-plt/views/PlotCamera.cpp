#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "PlotCamera.h"


namespace PH::LabCorePLT {

	glm::mat4 getPlotCameraProjectionMatrix(const PlotCamera& cam) {
		return glm::ortho(cam.domain.left, cam.domain.right, cam.domain.bottom, cam.domain.top, -100.0f, 100.0f);
	}

	glm::mat4 getPlotCameraViewMatrix(const PlotCamera& cam) {
		return glm::translate(glm::mat4(1.0f), { -cam.position.x, -cam.position.y, 0.0f });
	}

	glm::mat2 getScreenToWorldMatrix(const PlotCamera& cam, const glm::vec2 screensize) {
		return glm::mat2(glm::inverse(cam.view) * glm::inverse(cam.proj) * glm::ortho(0.0f, screensize.x, 0.0f, screensize.y));
	}

	glm::mat4 getWorldToScreenSpace(const PlotCamera& cam, const glm::vec2& screensize) {
		glm::mat4 screenspace = glm::ortho(0.0f, screensize.x, 0.0f, screensize.y, -100.0f, 100.0f);

		return glm::inverse(screenspace) * cam.proj * cam.view;
	}

	PlotCamera createPlotCamera(Engine::Box2D domain, glm::vec2 position) {
		PlotCamera r;
		r.domain = domain;
		r.position = position;

		r.proj = getPlotCameraProjectionMatrix(r);
		r.view = getPlotCameraViewMatrix(r);

		return r;
	}

	void resolvePlotCameraEvent(const PH::Platform::Event& e, PlotCamera* cam) {

	}

	void updateCamera(PlotCamera* cam) {

	}


}