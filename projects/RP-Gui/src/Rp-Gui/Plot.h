#pragma once
#include <Engine//Rendering.h>
#include <Engine/cppAPI/Rendering.hpp>
#include "Text.h"

namespace PH::RpGui {

	extern Engine::Renderer2D::Wrapper renderer2D;

	struct Box2D {

		real32 left;
		real32 bottom;
		real32 top;
		real32 right;

	};

	//WARNING: does the transform in the memory of the points array, changes the contents of the array! 
	bool32 drawLineStrip(PH::Base::Array<glm::vec2> points, glm::vec2 thickness, const glm::vec4& color, const glm::mat4& transform) {

		real32 depth = transform[3][2];

		glm::vec3 v1 = { transform[0] };
		glm::vec3 v2 = { transform[1] };
		glm::vec3 v3 = { transform[3].x, transform[3].y, 1.0f };

		glm::mat3 transform3 = glm::mat3(v1, v2, v3);

		for (auto& point : points) {
			point = glm::vec2(transform3 * glm::vec3(point, 1.0f));
		}

		return RpGui::renderer2D.drawLineStrip(points, color, thickness, depth);
	}

	bool32 drawLine(glm::vec2 v1, glm::vec2 v2, glm::vec4 color, glm::vec2 thickness, const glm::mat4& transform) {
		real32 depth = transform[3][2];

		glm::vec3 tv1 = { transform[0] };
		glm::vec3 tv2 = { transform[1] };
		glm::vec3 tv3 = { transform[3].x, transform[3].y, 1.0f };

		glm::mat3 transform3 = glm::mat3(tv1, tv2, tv3);

		glm::vec3 v1t = glm::vec3(glm::vec2(transform3 * glm::vec3(v1, 1.0f)), depth);
		glm::vec3 v2t = glm::vec3(glm::vec2(transform3 * glm::vec3(v2, 1.0f)), depth);

		return RpGui::renderer2D.drawLine(v1t, v2t, color, thickness);
	}

	//draws a linestrip transformed from the corresponding range to the region on the display as given by the parameters
	bool32 drawPlot(PH::Base::Array<glm::vec2> points, Box2D range, Box2D region, glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec2 thickness = { 1.0f, 1.0f }) {

		//transform from range to -1.0f, 1.0f
		real32 width = region.right - region.left;
		real32 heigth = region.top - region.bottom;

		glm::mat4 rangeToStdDev = glm::ortho(range.left, range.right, range.bottom, range.top);
		glm::mat4 rangeToStd = glm::scale(glm::mat4(1.0f), glm::vec3{ 0.5f, 0.5f, 1.0f }) * glm::translate(glm::mat4(1.0f), glm::vec3{ 1.0f, 1.0f, 0.0f }) * rangeToStdDev;
		//glm::mat4 rangeToStd = glm::ortho()

		glm::vec4 vx = { width, 0.0f, 0.0f, 0.0f };
		glm::vec4 vy = { 0.0f, heigth, 0.0f, 0.0f };
		glm::vec4 vz = { 0.0f, 0.0f, 1.0f, 0.0f };
		glm::vec4 translate = { region.left, region.bottom, 0.0f, 1.0f };

		glm::mat4 stdToRegion = glm::mat4(vx, vy, vz, translate);
		glm::mat4 rangeToRegion = stdToRegion * rangeToStd;

		return drawLineStrip(points, thickness, color, rangeToRegion);
	}

	bool32 contains(glm::vec2 point, Box2D box) {
		return point.x >= box.left && point.x <= box.right && point.y >= box.bottom && point.y <= box.top;
	}

	bool32 drawPlotScaleLines(Box2D range, Box2D region) {
		//transform from range to -1.0f, 1.0f
		real32 width = region.right - region.left;
		real32 heigth = region.top - region.bottom;

		glm::mat4 rangeToStdDev = glm::ortho(range.left, range.right, range.bottom, range.top);
		glm::mat4 rangeToStd = glm::scale(glm::mat4(1.0f), glm::vec3{ 0.5f, 0.5f, 1.0f }) * glm::translate(glm::mat4(1.0f), glm::vec3{ 1.0f, 1.0f, 0.0f }) * rangeToStdDev;

		glm::vec4 vx = { width, 0.0f, 0.0f, 0.0f };
		glm::vec4 vy = { 0.0f, heigth, 0.0f, 0.0f };
		glm::vec4 vz = { 0.0f, 0.0f, 1.0f, 0.0f };
		glm::vec4 translate = { region.left, region.bottom, 0.0f, 1.0f };

		glm::mat4 stdToRegion = glm::mat4(vx, vy, vz, translate);
		glm::mat4 rangeToRegion = stdToRegion * rangeToStd;

		real32 rangemult = 2.5f;

		int32 xorder = floor(log10((range.right - range.left) * rangemult)) - 1;
		int32 yorder = floor(log10((range.top - range.bottom) * rangemult)) - 1;

		real32 xstep = pow(10.0f, xorder);
		real32 ystep = pow(10.0f, yorder);

		real32 xstart = floor((range.left / xstep)) * xstep;
		real32 ystart = floor((range.bottom / ystep)) * ystep;

		real32 slinebrightness = 0.04f;
		real32 linebrightness = 0.2f;

		real32 sxstep = xstep * 0.2f;
		real32 systep = ystep * 0.2f;

		//draw lines in x direction
		for (real32 x = xstart; x <= range.right; x += sxstep) {
			drawLine({ x, range.bottom }, { x, range.top }, { glm::vec3(slinebrightness), 1.0f}, {1.0f, 1.0f}, rangeToRegion);
		}

		//draw lines in y direction
		for (real32 y = ystart; y <= range.top; y += systep) {
			drawLine({ range.left, y }, { range.right, y }, { glm::vec3(slinebrightness), 1.0f}, {1.0f, 1.0f}, rangeToRegion);
		}

		//draw large lines in x direction
		//draw lines in x direction
		for (real32 x = xstart; x <= range.right; x += xstep) {
			drawLine({ x, range.bottom }, { x, range.top }, { glm::vec3(linebrightness), 1.0f}, {1.0f, 1.0f}, rangeToRegion);
		}

		//draw large lines in y direction
		//draw lines in y direction
		for (real32 y = ystart; y <= range.top; y += ystep) {
			drawLine({ range.left, y }, { range.right, y }, { glm::vec3(linebrightness), 1.0f}, {1.0f, 1.0f}, rangeToRegion);
		}



		if (range.top > 0.0f && range.bottom < 0.0f) {
			drawLine({ range.left, 0.0f }, { range.right, 0.0f }, { 0.8f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, rangeToRegion);
		}

		if (range.left < 0.0f && range.right > 0.0f) {
			drawLine({ 0.0f, range.bottom }, { 0.0f, range.top }, { 0.8f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, rangeToRegion);
		}


		return true;
	}

	bool32 drawPlotScaleValues(Box2D range, Box2D region, Font* font, real32 scale = 1.0f) {
		//transform from range to -1.0f, 1.0f
		real32 width = region.right - region.left;
		real32 heigth = region.top - region.bottom;

		glm::mat4 rangeToStdDev = glm::ortho(range.left, range.right, range.bottom, range.top);
		glm::mat4 rangeToStd = glm::scale(glm::mat4(1.0f), glm::vec3{ 0.5f, 0.5f, 1.0f }) * glm::translate(glm::mat4(1.0f), glm::vec3{ 1.0f, 1.0f, 0.0f }) * rangeToStdDev;

		glm::vec4 vx = { width, 0.0f, 0.0f, 0.0f };
		glm::vec4 vy = { 0.0f, heigth, 0.0f, 0.0f };
		glm::vec4 vz = { 0.0f, 0.0f, 1.0f, 0.0f };
		glm::vec4 translate = { region.left, region.bottom, 0.0f, 1.0f };

		glm::mat4 stdToRegion = glm::mat4(vx, vy, vz, translate);
		glm::mat4 rangeToRegion = stdToRegion * rangeToStd;

		real32 rangemult = 2.5f;

		int32 xorder = floor(log10((range.right - range.left) * rangemult)) - 1;
		int32 yorder = floor(log10((range.top - range.bottom) * rangemult)) - 1;

		real32 xstep = pow(10.0f, xorder);
		real32 ystep = pow(10.0f, yorder);

		real32 xstart = floor((range.left / xstep)) * xstep;
		real32 ystart = floor((range.bottom / ystep)) * ystep;

		glm::vec2 padding = { 5.0f, 5.0f };

		//draw lines in x direction
		for (real32 x = xstart; x <= range.right; x += xstep) {
			char buffer[64];
			snprintf(buffer, 64, "%4.1f", x);

			glm::vec4 position = rangeToRegion * glm::vec4{ x, range.bottom, 0.0f, 1.0f };
			drawText(font, buffer, glm::vec2(position) + padding, scale);
		}

		//draw lines in y direction
		for (real32 y = ystart; y <= range.top; y += ystep) {

			char buffer[64];
			snprintf(buffer, 64, "%4.1f", y);

			glm::vec4 position = rangeToRegion * glm::vec4{ range.left, y, 0.0f, 1.0f };
			drawText(font, buffer, glm::vec2(position) + padding, scale);
		}

		return true;
	}

	void drawTextureQuadBottomLeft(glm::vec3 bottomleft, glm::vec2 scale, PH::Platform::GFX::Texture texture) {
		RpGui::renderer2D.drawTexturedQuad({ bottomleft.x + scale.x, bottomleft.y + scale.y, bottomleft.z }, { scale.x, scale.y }, texture);
	}

}