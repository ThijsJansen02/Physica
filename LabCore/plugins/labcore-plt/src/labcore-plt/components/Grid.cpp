#include "Grid.h"
#include <LabCore/LabCoreAPI.h>
#include "../views/PlotView.h"

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

	glm::vec4 major_color = glm::vec4(0.3f);
	glm::vec4 minor_color = glm::vec4(0.1f);
	glm::vec4 minor_text_color = glm::vec4(1.0f);

	real32 gridthickness = 0.7f;
	constexpr const char* prefixes[] = {"p", "n", "u", "m", "", "k", "M", "G", "T", "P"};

	real32 majorlayer = 1.0f;
	real32 minorlayer = 0.0f;
	real32 textlayer = 2.0f;

	real64 calculateLineGranularity(real32 min_gridspacing, real32 ws_viewportsize, real32 ss_viewportsize) {
		real32 min_gridspace_ws = (min_gridspacing / ss_viewportsize) * ws_viewportsize;
		int32 gridspace_order_x_ws = calcOrder(min_gridspace_ws) + 1;
		return Base::pow_b_Uint64_exp_Int64(10, gridspace_order_x_ws);
	}

	glm::vec2 screenSpaceToWorldSpace(const glm::vec2& v, const glm::vec2& ws_viewportsize, const glm::vec2& ss_viewportsize) {
		return (v / ss_viewportsize) * ws_viewportsize;
	}

	void drawLinearGrid_Y(LabCore::Scene* scene, LabCore::Scene::RenderProperties* renderproperties) {
		//domain scale
		glm::mat4 inverseviewproj = glm::inverse(renderproperties->camera.projection * renderproperties->camera.view);
		glm::vec2 viewport_ws = 2.0f * glm::mat2(inverseviewproj) * glm::vec2(1.0f);

		//minimal spacing between grid lines
		real32 min_gridspacing_major_px = 70.0f;
		real32 min_gridspacing_minor_px = 7.0f;

		real32 ygranularity_major = calculateLineGranularity(min_gridspacing_major_px, viewport_ws.y, renderproperties->viewportsize.y);
		real32 ygranularity_minor = calculateLineGranularity(min_gridspacing_minor_px, viewport_ws.y, renderproperties->viewportsize.y);

		//could probably make a matrix out of this but is only called once per frame so not performance critical
		real64 left = (inverseviewproj * glm::vec4(-1.0f, 0.0, 0.0, 1.0f)).x;
		real64 right = (inverseviewproj * glm::vec4(1.0f, 0.0, 0.0, 1.0f)).x;
		real64 top = (inverseviewproj * glm::vec4(0.0f, 1.0, 0.0, 1.0f)).y;
		real64 bottom = (inverseviewproj * glm::vec4(0.0f, -1.0, 0.0, 1.0f)).y;

		real64 ystart = Base::floorReal64(bottom / ygranularity_major) * ygranularity_major;

		Engine::BatchRenderer2D* renderer = LabCore::getRenderer2D();
		
		//draw minor gridlines
		real64 yptr = ystart;
		while (yptr < top) {
			renderer->drawLine({ left, yptr }, { right, yptr }, minorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), minor_color);
			yptr += ygranularity_minor;
		}

		//draw major grid lines
		yptr = ystart;
		while (yptr < top) {
			renderer->drawLine({ left, yptr }, { right, yptr }, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), major_color);
			yptr += ygranularity_major;
		}

		renderer->drawLine({ left, 0.0f }, { right, 0.0f }, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), glm::vec4{ 0.5f, 0.0f, 0.0f, 1.0f });

		auto* font = LabCore::getAssetLibrary()->getAssetByReference<Engine::Font>("defaultfont");
		if (!font) {
			return;
		}

		char buffer[64];
		const char* unit = getCurrentPlotViewSettings()->y_unit;

		real32 textscale_ss = 12.0f;

		glm::vec2 textscale_ws = { (textscale_ss / renderproperties->viewportsize.x) * viewport_ws.x, (textscale_ss / renderproperties->viewportsize.y) * viewport_ws.y };
		yptr = ystart;
		while (yptr < top) {

			int32 scaleOrder = calcOrder(abs(ygranularity_major));
			if (scaleOrder < 0) {
				scaleOrder -= 2;
			}
			int32 deg = scaleOrder / 3;

			int32 prefixindex = deg + 4;

			real64 mul = Base::pow_b_Uint64_exp_Int64(10, deg * 3);

			const char* prefix = "";
			if (prefixindex > 0 && prefixindex < ARRAYSIZE(prefixes)) {
				prefix = prefixes[prefixindex];
			}
			sprintf_s(buffer, 64, "%4.1f%s%s", yptr / mul, prefix, unit);
			renderer->drawText(buffer, glm::vec3{ left, yptr, textlayer } + glm::vec3(screenSpaceToWorldSpace({ 4.0f, 4.0f }, viewport_ws, renderproperties->viewportsize), 0.0f), textscale_ws, font);

			yptr += ygranularity_major;
		}
	}

	void drawLinearGrid_X(LabCore::Scene* scene, LabCore::Scene::RenderProperties* renderproperties) {
		//domain scale
		glm::mat4 inverseviewproj = glm::inverse(renderproperties->camera.projection * renderproperties->camera.view);
		glm::vec2 viewport_ws = 2.0f * glm::mat2(inverseviewproj) * glm::vec2(1.0f);


		//minimal spacing between grid lines
		real32 min_gridspacing_major_px = 70.0f;
		real32 min_gridspacing_minor_px = 7.0f;

		real32 xgranularity_major = calculateLineGranularity(min_gridspacing_major_px, viewport_ws.x, renderproperties->viewportsize.x);
		real32 xgranularity_minor = calculateLineGranularity(min_gridspacing_minor_px, viewport_ws.x, renderproperties->viewportsize.x);
		

		//could probably make a matrix out of this but is only called once per frame so not performance critical
		real64 left = (inverseviewproj * glm::vec4(-1.0f, 0.0, 0.0, 1.0f)).x;
		real64 right = (inverseviewproj * glm::vec4(1.0f, 0.0, 0.0, 1.0f)).x;
		real64 top = (inverseviewproj * glm::vec4(0.0f, 1.0, 0.0, 1.0f)).y;
		real64 bottom = (inverseviewproj * glm::vec4(0.0f, -1.0, 0.0, 1.0f)).y;

		real64 xstart = Base::floorReal64(left / xgranularity_major) * xgranularity_major;
		
		Engine::BatchRenderer2D* renderer = LabCore::getRenderer2D();

		//draw minor lines
		real64 xptr = xstart;
		while (xptr < right) {
			renderer->drawLine({ xptr, bottom }, { xptr, top }, minorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), minor_color);
			xptr += xgranularity_minor;
		}

		//draw major lines
		xptr = xstart;
		while (xptr < right) {
			renderer->drawLine({ xptr, bottom }, { xptr, top }, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), major_color);
			xptr += xgranularity_major;
		}

		renderer->drawLine({ 0.0f, bottom }, { 0.0f, top}, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), glm::vec4{ 0.0f, 0.5f, 0.0f, 1.0f });

		//draw markers

		auto* font = LabCore::getAssetLibrary()->getAssetByReference<Engine::Font>("defaultfont");
		if (!font) {
			return;
		}

		char buffer[64];
		const char* unit = getCurrentPlotViewSettings()->x_unit;

		real32 textscale_ss = 12.0f;

		glm::vec2 textscale_ws = { (textscale_ss / renderproperties->viewportsize.x) * viewport_ws.x, (textscale_ss / renderproperties->viewportsize.y) * viewport_ws.y };

		xptr = xstart;
		while (xptr < right) {

			int32 scaleOrder = calcOrder(abs(xgranularity_major));
			if (scaleOrder < 0) {
				scaleOrder -= 2;
			}
			int32 deg = scaleOrder / 3;

			int32 prefixindex = deg + 4;

			real64 mul = Base::pow_b_Uint64_exp_Int64(10, deg * 3);

			const char* prefix = "";
			if (prefixindex > 0 && prefixindex < ARRAYSIZE(prefixes)) {
				prefix = prefixes[prefixindex];
			}
			sprintf_s(buffer, 64, "%4.1f%s%s", xptr / mul, prefix, unit);
			renderer->drawText(buffer, glm::vec3{ xptr, bottom, textlayer } + glm::vec3(screenSpaceToWorldSpace({ 4.0f, 4.0f }, viewport_ws, renderproperties->viewportsize), 0.0f), textscale_ws, font);

			xptr += xgranularity_major;
		}
	}

	void sprintLogMarker(char* buffer, sizeptr buffersize, real64 logvalue, const char* unit) {
		int32 scaleOrder = logvalue;
		if (scaleOrder < 0) {
			scaleOrder -= 2;
		}
		int32 deg = scaleOrder / 3;
		int32 prefixindex = deg + 4;
		real64 mul = Base::pow_b_Uint64_exp_Int64(10, deg * 3);

		//should add ability to scale text in Y and X, to have all text size the same size

		const char* prefix = "";
		if (prefixindex > 0 && prefixindex < ARRAYSIZE(prefixes)) {
			prefix = prefixes[prefixindex];
		}
		sprintf_s(buffer, 64, "%4.2f%s%s", Base::pow_b_Uint64_exp_Int64(10, (int64)logvalue) / mul, prefix, unit);
	}

	void sprintLinearMarker(char* buffer, sizeptr buffersize, real64 value, const char* unit) {
		int32 scaleOrder = calcOrder(value);
		if (scaleOrder < 0) {
			scaleOrder -= 2;
		}
		int32 deg = scaleOrder / 3;
		int32 prefixindex = deg + 4;
		real64 mul = Base::pow_b_Uint64_exp_Int64(10, deg * 3);

		//should add ability to scale text in Y and X, to have all text size the same size

		const char* prefix = "";
		if (prefixindex > 0 && prefixindex < ARRAYSIZE(prefixes)) {
			prefix = prefixes[prefixindex];
		}
		sprintf_s(buffer, 64, "%4.2f%s%s", value / mul, prefix, unit);
	}

	void drawLogGrid_X(LabCore::Scene* scene, LabCore::Scene::RenderProperties* renderproperties) {

		//in the log grid the domain is expressed in log units, meaning if left = -10 it corresponds with 10^-10

		//domain scale
		glm::mat4 inverseviewproj = glm::inverse(renderproperties->camera.projection * renderproperties->camera.view);
		glm::vec2 viewport_ws = 2.0f * glm::mat2(inverseviewproj) * glm::vec2(1.0f);

		//could probably make a matrix out of this but is only called once per frame so not performance critical
		real64 logleft = (inverseviewproj * glm::vec4(-1.0f, 0.0, 0.0, 1.0f)).x;
		real64 logright = (inverseviewproj * glm::vec4(1.0f, 0.0, 0.0, 1.0f)).x;
		real64 logtop = (inverseviewproj * glm::vec4(0.0f, 1.0, 0.0, 1.0f)).y;
		real64 logbottom = (inverseviewproj * glm::vec4(0.0f, -1.0, 0.0, 1.0f)).y;
		
		real64 xstart = Base::floorReal64(logleft);
		real64 xptr = xstart;

		Engine::BatchRenderer2D* renderer = LabCore::getRenderer2D();

		//draw major lines;
		while (xptr < logright) {
			renderer->drawLine({ xptr, logbottom }, { xptr, logtop }, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), major_color);

			//the linear value of the xptr
			real64 linearxptr = Base::powReal64(10.0f, xptr);

			//draw minor lines;
			real64 linearminorptr = linearxptr;
			for (uint32 i = 0; i < 8; i++) {
				linearminorptr += linearxptr;

				real64 logminorptr = log10(linearminorptr);
				renderer->drawLine({ logminorptr, logbottom }, { logminorptr, logtop }, minorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), minor_color);
			}

			xptr += 1.0f;
		}

		renderer->drawLine({ 0.0f, logbottom }, { 0.0f, logtop }, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));

		//draw minor lines

		//draw markers
		auto* font = LabCore::getAssetLibrary()->getAssetByReference<Engine::Font>("defaultfont");
		if (!font) {
			return;
		}

		char buffer[64];
		const char* unit = getCurrentPlotViewSettings()->x_unit;

		real32 textscale_ss = 12.0f;

		glm::vec2 textscale_ws = { (textscale_ss / renderproperties->viewportsize.x) * viewport_ws.x, (textscale_ss / renderproperties->viewportsize.y) * viewport_ws.y };

		xptr = xstart;
		while (xptr < logright) {
			sprintLogMarker(buffer, 64, xptr, unit);
			renderer->drawText(buffer, glm::vec3{ xptr, logbottom, textlayer } + glm::vec3(screenSpaceToWorldSpace({ 4.0f, 4.0f }, viewport_ws, renderproperties->viewportsize), 0.0f), textscale_ws, font);
			
			//the linear value of the xptr
			real64 linearxptr = Base::powReal64(10.0f, xptr);
			
			glm::vec2 ws = screenSpaceToWorldSpace({ 700.0f, 0.0 }, viewport_ws, renderproperties->viewportsize);
			if (ws.x < 1.0) {
				//draw minor lines;
				real64 linearminorptr = linearxptr;
				for (uint32 i = 0; i < 8; i++) {
					linearminorptr += linearxptr;
					real64 logminorptr = log10(linearminorptr);

					sprintLinearMarker(buffer, 64, linearminorptr, unit);
					renderer->drawText(buffer, glm::vec3{ logminorptr, logbottom, textlayer } + glm::vec3(screenSpaceToWorldSpace({ 4.0f, 4.0f }, viewport_ws, renderproperties->viewportsize), 0.0f), textscale_ws, font, minor_text_color);

					//renderer->drawLine({ logminorptr, logbottom }, { logminorptr, logtop }, minorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), minor_color);
				}
			}
			xptr += 1.0f;
		}
	}

	void drawLogGrid_Y(LabCore::Scene* scene, LabCore::Scene::RenderProperties* renderproperties) {

		//in the log grid the domain is expressed in log units, meaning if left = -10 it corresponds with 10^-10

		//domain scale
		glm::mat4 inverseviewproj = glm::inverse(renderproperties->camera.projection * renderproperties->camera.view);
		glm::vec2 viewport_ws = 2.0f * glm::mat2(inverseviewproj) * glm::vec2(1.0f);

		//could probably make a matrix out of this but is only called once per frame so not performance critical
		real64 logleft = (inverseviewproj * glm::vec4(-1.0f, 0.0, 0.0, 1.0f)).x;
		real64 logright = (inverseviewproj * glm::vec4(1.0f, 0.0, 0.0, 1.0f)).x;
		real64 logtop = (inverseviewproj * glm::vec4(0.0f, 1.0, 0.0, 1.0f)).y;
		real64 logbottom = (inverseviewproj * glm::vec4(0.0f, -1.0, 0.0, 1.0f)).y;

		real64 ystart = Base::floorReal64(logbottom);
		real64 yptr = ystart;

		Engine::BatchRenderer2D* renderer = LabCore::getRenderer2D();

		//draw major lines;
		while (yptr < logtop) {
			renderer->drawLine({logleft , yptr}, { logright, yptr}, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), major_color);

			//the linear value of the xptr
			real64 linearyinc = Base::powReal64(10.0f, yptr);

			//draw minor lines;
			real64 linearminorptr = linearyinc;
			for (uint32 i = 0; i < 8; i++) {
				linearminorptr += linearyinc;

				real64 logminorptr = log10(linearminorptr);
				renderer->drawLine({ logleft, logminorptr }, { logright, logminorptr}, minorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), minor_color);
			}

			yptr += 1.0f;
		}

		renderer->drawLine({ logleft, 0.0f}, { logright, 0.0f}, majorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));


		//draw markers
		auto* font = LabCore::getAssetLibrary()->getAssetByReference<Engine::Font>("defaultfont");
		if (!font) {
			return;
		}

		char buffer[64];
		const char* unit = getCurrentPlotViewSettings()->x_unit;

		real32 textscale_ss = 12.0f;

		glm::vec2 textscale_ws = { (textscale_ss / renderproperties->viewportsize.x) * viewport_ws.x, (textscale_ss / renderproperties->viewportsize.y) * viewport_ws.y };

		yptr = ystart;
		while (yptr < logtop) {
			sprintLogMarker(buffer, 64, yptr, unit);
			renderer->drawText(buffer, glm::vec3{logleft, yptr, textlayer } + glm::vec3(screenSpaceToWorldSpace({ 4.0f, 4.0f }, viewport_ws, renderproperties->viewportsize), 0.0f), textscale_ws, font);

			//the linear value of the xptr
			real64 linearyinc = Base::powReal64(10.0f, yptr);

			glm::vec2 ws = screenSpaceToWorldSpace({ 0.0, 300.0 }, viewport_ws, renderproperties->viewportsize);
			if (ws.y < 1.0) {
				//draw minor lines;
				real64 linearminorptr = linearyinc;
				for (uint32 i = 0; i < 8; i++) {
					linearminorptr += linearyinc;
					real64 logminorptr = log10(linearminorptr);

					sprintLinearMarker(buffer, 64, linearminorptr, unit);
					renderer->drawText(buffer, glm::vec3{logleft , logminorptr, textlayer } + glm::vec3(screenSpaceToWorldSpace({ 4.0f, 4.0f }, viewport_ws, renderproperties->viewportsize), 0.0f), textscale_ws, font, minor_text_color);

					//renderer->drawLine({ logminorptr, logbottom }, { logminorptr, logtop }, minorlayer, gridthickness, glm::mat2(renderproperties->camera.worldtoscreen), minor_color);
				}
			}
			yptr += 1.0f;
		}
	}

	bool32 draw(LabCore::Scene* scene, LabCore::Scene::RenderProperties* renderproperties) {
		Engine::BatchRenderer2D* renderer = LabCore::getRenderer2D();

		if (!renderproperties->camera.orthographic) {
			return false;
		}

		PlotView::Settings* settings = getCurrentPlotViewSettings();

		if (!settings->logx) {
			drawLinearGrid_X(scene, renderproperties);
		}
		else {
			drawLogGrid_X(scene, renderproperties);
		}

		if (!settings->logy) {
			drawLinearGrid_Y(scene, renderproperties);
		}
		else {
			drawLogGrid_Y(scene, renderproperties);
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