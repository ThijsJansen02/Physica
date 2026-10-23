
#include "MaterialView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Font.h>

#include "../assets/Texture.h"
#include "../DefferredRenderer.h"

namespace PH::LabCore3D {

	Engine::AssetIdentifier* assetSelecter(const char* label, Engine::AssetIdentifier* selected, Base::Array<Engine::AssetIdentifier*> options, const char* assetstr) {

		Engine::AssetIdentifier* newselected = selected;
		
		if(ImGui::BeginCombo(label, selected ? selected->references[0].getC_Str() : assetstr)) {

			for (auto* asset : options) {
				if (ImGui::Selectable(asset->references[0].getC_Str(), asset == selected)) {
					newselected = asset;
				}
			}

			ImGui::EndCombo();
		}

		return newselected;
	}


	PH::bool32 drawMaterialView(MaterialView* view) {
		
		Engine::AssetLibrary* assets = LabCore::getAssetLibrary();
		Engine::ArrayList<Engine::AssetIdentifier*> pipelines = assets->getAllIdentifiersOfType<GraphicsPipeline>();

		view->currentpipe = assetSelecter("graphics pipeline", view->currentpipe, pipelines.getArray(), "select pipeline");


		if (view->currentpipe) {
			if(view->currentpipe->status == Engine::LOADED) {
				GraphicsPipeline* p = (GraphicsPipeline*)view->currentpipe->assetdata;

				if (ImGui::Button("reload")) {
					reloadGraphicsPipelineFromGLSL(p);
				}

				Engine::ArrayList<Engine::AssetIdentifier*> textureoptions = assets->getAllIdentifiersOfType<Texture>();
				for (auto& res : p->resources) {
					if (res.type == Platform::GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
						auto* old = res.texture;
						res.texture = assetSelecter(res.name.getC_Str(), res.texture, textureoptions.getArray(), "select texture");
						if (old != res.texture) {
							revalidateDescriptorSet(p);
						}
					}

				}

			}
		}


		Engine::ArrayList<Engine::AssetIdentifier*>::destroy(&pipelines);
		return true;
	}

	PH::bool32 updateMaterialView(MaterialView* view) {
		return true;
	}

	PH::bool32 instantiateMaterialView(MaterialView* view) {

		view->currentpipe = nullptr;

		return true;
	}

	PH::bool32 destroyMaterialView(MaterialView* data) {

		return true;
	}

	PH::bool32 materialViewOnEvent(MaterialView* data, const PH::Platform::Event& e) {
		return true;
	}

	PH::bool32 deserializeMaterialView(MaterialView* data, const YAML::Node& node) {

		auto* asset = LabCore::getAssetLibrary();

		if (node["selected"]) {
			data->currentpipe = asset->getAssetIdentifier<GraphicsPipeline>(node["selected"].as<Engine::AssetID>());
		}

		return true;
	}

	PH::bool32 serializeMaterialView(MaterialView* data, YAML::Emitter& out) {

		out << YAML::BeginMap;

		if (data->currentpipe) {
			out << YAML::Key << "selected" << YAML::Value << data->currentpipe->assetid;
		}

		out << YAML::EndMap;

		return true;
	}

	LabCore::ViewDescription createMaterialViewDescription() {
		LabCore::ViewDescription d{};
		d.draw_ = (LabCore::ViewDrawFunction)drawMaterialView;
		d.onUpdate_ = (LabCore::ViewOnUpdateFunction)updateMaterialView;
		d.onEvent_ = (LabCore::ViewOnEventFunction)materialViewOnEvent;

		d.destroy_ = (LabCore::ViewDestroyFunction)destroyMaterialView;
		d.instantiate_ = (LabCore::ViewInstantiateFunction)instantiateMaterialView;

		d.windowpadding = { 5,5 };
		d.windowflags = 0;

		d.deserialize_ = (LabCore::ViewDeserializeFunction)deserializeMaterialView;
		d.serialize_ = (LabCore::ViewSerializeFunction)serializeMaterialView;

		d.name = Engine::String::create("Material View");
		d.instancedatasize = sizeof(MaterialView);

		return d;
	}
}