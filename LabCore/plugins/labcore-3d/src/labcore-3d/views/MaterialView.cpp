
#include "MaterialView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Font.h>
#include <Engine/Events.h>
#include <Engine/imgui/extensions.h>

#include "../assets/Texture.h"
#include "../DefferredRenderer.h"
#include "../LabCore3D.h"

namespace PH::LabCore3D {

	

	bool32 drawUniformBufferMembers(UniformBufferResource* ubr, GraphicsPipeline* pipe) {
		real32 dragspeed = 0.01;
		bool32 dirty = false;

		uint8* bufferdata = (uint8*)pipe->userbufferlocaldata;

		for (auto& bm : ubr->members) {
			if (bm.type == LC_BUFFER_MEMBER_FLOAT) {
				if (bm.name.contains("_color")) {
					if (bm.vecsize == 3) {
						dirty |= ImGui::ColorEdit3(bm.name.getC_Str(), (float*)(bufferdata + bm.userbufferoffset));
					}

					if (bm.vecsize == 4) {
						dirty |= ImGui::ColorEdit4(bm.name.getC_Str(), (float*)(bufferdata + bm.userbufferoffset));
					}
				}
				else {
					real32 speed = *(real32*)(bufferdata + bm.userbufferoffset) * dragspeed;
					dirty |= ImGui::DragScalarN(bm.name.getC_Str(), ImGuiDataType_Float, (bufferdata + bm.userbufferoffset), bm.vecsize, speed);
				}
			}

			if (bm.type == LC_BUFFER_MEMBER_UINT) {
				dirty |= ImGui::InputScalarN(bm.name.getC_Str(), ImGuiDataType_U32, (bufferdata + bm.userbufferoffset), bm.vecsize);
			}
		}
		return dirty;
	}

	PH::bool32 drawMaterialView(MaterialView* view) {
		
		Engine::AssetLibrary* assets = LabCore::getAssetLibrary();
		Engine::ArrayList<Engine::AssetIdentifier*> pipelines = assets->getAllIdentifiersOfType<GraphicsPipeline>();

		view->currentpipe = ImGuiext::assetSelecter("graphics pipeline", view->currentpipe, pipelines.getArray(), "select pipeline");


		if (view->currentpipe) {

			//set current directory to home or projectdir depending on the asset
			if (view->currentpipe->flags & Engine::PH_ASSET_SERIALIZABLE) {
				PH::Platform::setCurrentDirectory(LabCore::getCurrentProjectDir());
			}
			else {
				PH::Platform::setCurrentDirectory(LabCore3D::home.getSubString());
			}

			if(view->currentpipe->status == Engine::LOADED) {
				GraphicsPipeline* p = (GraphicsPipeline*)view->currentpipe->assetdata;

				if (ImGui::Button("reload")) {
					reloadGraphicsPipelineFromGLSL(p);


					LabCore::getAssetLibrary()->SerializeAsset<PH::LabCore3D::GraphicsPipeline>(view->currentpipe, view->currentpipe->filepath.getC_Str());
				}

				Engine::ArrayList<Engine::AssetIdentifier*> textureoptions = assets->getAllIdentifiersOfType<Texture>();
				
				uint8* bufferdata = (uint8*)p->userbufferlocaldata;
				bool ubosdirty = false;
				bool texturesdirty = false;

				for (auto& res : p->resources) {
					if (res.type == Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
						
						UniformBufferResource* ubr = res.uniformbufferresource;
						ubosdirty |= drawUniformBufferMembers(ubr, p);
					}

					if (res.type == Platform::GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
						auto* old = res.texture;
						res.texture = ImGuiext::assetSelecter(res.name.getC_Str(), res.texture, textureoptions.getArray(), "select texture");
						if (old != res.texture) {
							texturesdirty = true;
						}
					}
				}

				if (ubosdirty) {
					revalidateUniformBuffers(p, &view->stagingbuffer);
				}
				if (texturesdirty) {
					LabCore::getAssetLibrary()->SerializeAsset<PH::LabCore3D::GraphicsPipeline>(view->currentpipe, view->currentpipe->filepath.getC_Str());
					revalidateTextures(p);
				}
			}
		}


		Engine::ArrayList<Engine::AssetIdentifier*>::destroy(&pipelines);
		return true;
	}

	PH::bool32 updateMaterialView(MaterialView* view) {
		return true;
	}

#define MATERIALVIEW_STAGING_BUFFER_SIZE 1024

	using namespace PH::Platform;

	PH::bool32 instantiateMaterialView(MaterialView* view) {

		view->currentpipe = nullptr;
		view->stagingbuffer = Engine::DynamicGPUBuffer<uint8>::create(MATERIALVIEW_STAGING_BUFFER_SIZE, GFX::BUFFER_USAGE_TRANSFER_SRC_BIT);

		return true;
	}

	PH::bool32 destroyMaterialView(MaterialView* data) {

		return true;
	}

	PH::bool32 materialViewOnEvent(MaterialView* data, const PH::Platform::Event& e) {
		
		if (e.type == PH_EVENT_TYPE_KEY_PRESSED) {
			if (e.lparam == PH_KEY_S && Engine::Events::isKeyPressed(PH_CONTROL)) {
				//set current directory to home or projectdir depending on the asset
				if (data->currentpipe->flags & Engine::PH_ASSET_SERIALIZABLE) {
					PH::Platform::setCurrentDirectory(LabCore::getCurrentProjectDir());
				}
				else {
					PH::Platform::setCurrentDirectory(LabCore3D::home.getSubString());
				}

				LabCore::getAssetLibrary()->SerializeAsset<GraphicsPipeline>(data->currentpipe, data->currentpipe->filepath.getC_Str());
			}
		}
		
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