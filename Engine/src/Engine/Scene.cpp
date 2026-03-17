#include <entt/entt.hpp>
#include "Engine.h"

#define PH_SCENE_IMPLEMENTATION
#include "Scene.h"
#include "Components.h"
#include <vector>
#include "YamlExtensions.h"
#include "assets/Texture.h"
#include "AssetLibrary.h"

using namespace PH::Platform;

namespace PH::Engine::Assets {

#define MAX_LIGHTS 12


#pragma pack(push, 1)
	//vec4 because of padding
	struct PointLight {
		glm::vec4 position;
		glm::vec4 tint;
		real32 brightness;
		real32 linearfallof;
		real32 quadraticfallof;;
		real32 padding;
	};

	//uses vec4s because of padding
	struct DirectionalLight {
		glm::vec4 direction;
		glm::vec4 tint;
	};

	struct SceneBufferData {
		glm::mat4 projection;
		glm::mat4 view;
		PointLight lights[MAX_LIGHTS];
		DirectionalLight skylight;
		uint32 lightcount;
	};

#pragma pack(pop)

	volatile Platform::GFX::DescriptorSetLayout Scene::descriptorsetlayout;

	GFX::DescriptorSetLayout createSceneDescriptorSetLayout() {

		GFX::DescriptorBinding ubobinding;
		ubobinding.binding = Renderer2D::SCENE_UBO_BINDING;
		ubobinding.descriptorcount = 1;
		ubobinding.shaderstages = Platform::GFX::SHADER_STAGE_VERTEX_BIT | Platform::GFX::SHADER_STAGE_FRAGMENT_BIT;
		ubobinding.type = Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		GFX::DescriptorBinding skyboxbinding;
		skyboxbinding.binding = Renderer2D::SCENE_SKYBOX_BINDING;
		skyboxbinding.descriptorcount = 1;
		skyboxbinding.shaderstages = Platform::GFX::SHADER_STAGE_FRAGMENT_BIT;
		skyboxbinding.type = Platform::GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		GFX::DescriptorSetLayoutCreateinfo createinfo{};
		GFX::DescriptorBinding bindings[2] = { ubobinding, skyboxbinding };

		createinfo.bindings = { bindings, 2 };

		GFX::DescriptorSetLayout result;
		GFX::createDescriptorSetLayouts(&createinfo, &result, 1);
		return result;
	}

	GFX::DescriptorSetLayout createSceneDescriptorSet(Scene::Instance* instance) {

		GFX::DescriptorSetCreateinfo createinfo{};
		createinfo.dynamic = true;
		createinfo.layout = Scene::getSceneDescriptorSetLayout(instance);

		GFX::DescriptorSet result;

		GFX::createDescriptorSets(&createinfo, &result, 1);

		GFX::DescriptorBufferInfo bufferinfo;
		bufferinfo.buffer = instance->scenebuffer;
		bufferinfo.offset = 0;
		bufferinfo.range = sizeof(SceneBufferData);

		GFX::DescriptorWrite write{};
		write.dynamicwrite = false;
		write.dstset = result;
		write.type = GFX::DescriptorType::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.dstbinding = Renderer2D::SCENE_UBO_BINDING;
		write.descriptorinfo = &bufferinfo;
		write.arrayelement = 0;
		write.descriptorcount = 1;

		GFX::updateDescriptorSets(&write, 1);

		if (instance->skybox) {
			GFX::DescriptorImageInfo imageinfo;
			imageinfo.texture = instance->skybox->base;

			write;
			write.dynamicwrite = false;
			write.dstset = result;
			write.type = GFX::DescriptorType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.dstbinding = Renderer2D::SCENE_SKYBOX_BINDING;
			write.descriptorinfo = &imageinfo;
			write.arrayelement = 0;
			write.descriptorcount = 1;

			GFX::updateDescriptorSets(&write, 1);
		}
		return result;
	}

	bool32 Scene::setSkyBox(Assets::CubeMap* map, Assets::MaterialInstance* shader, Scene::Instance* instance) {

		GFX::DescriptorImageInfo imageinfo;
		instance->skybox = map;

		imageinfo.texture = instance->skybox->base;

		GFX::DescriptorWrite write{};

		write.dynamicwrite = false;
		write.dstset = instance->scenedescriptor;
		write.type = GFX::DescriptorType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.dstbinding = Renderer2D::SCENE_SKYBOX_BINDING;
		write.descriptorinfo = &imageinfo;
		write.arrayelement = 0;
		write.descriptorcount = 1;

		GFX::updateDescriptorSets(&write, 1);

		instance->skyboxshader = shader;

		return true;
	}

	Scene::Instance* Scene::createInstance() {

		Instance* instance = (Instance*)Engine::Allocator::alloc(sizeof(Instance));

		instance->skybox = nullptr;
		new ((void*)&instance->registry) entt::basic_registry<entt::entity, entt_allocator<entt::entity>>();

		instance->scenebuffer = createDynamicUniformBuffer(sizeof(SceneBufferData));
		instance->scenedescriptor = createSceneDescriptorSet(instance);
		instance->location = String::create("res/assets/scenes/NewScene.phscene");

		return instance;
	};

	Scene::Entity PH_ENGINE_API Scene::createEntity(Scene::Instance* instance, const char* name, uint64 uuid) {

		EntityHandle ent = (EntityHandle)instance->registry.create();

		//add a transform component;
		TransformComponent transform;
		transform.global = glm::mat4(1.0f);
		transform.local = glm::mat4(1.0f);

		addComponent<TransformComponent>(ent, transform, instance);

		TagComponent tag{};
		tag.tag = String::create(name);
		tag.uuid = uuid;

		addComponent<TagComponent>(ent, tag, instance);

		return { instance, ent };
	}

	bool32 PH_ENGINE_API Scene::destroyEntity(EntityHandle entity, Instance* instance) {
		instance->registry.destroy((entt::entity)entity);
		return true;
	}

	bool32 Scene::pushToRenderer2D(Renderer2D::Context* rendercontext, Scene::Instance* instance) {

		auto group = instance->registry.group<TransformComponent, QuadComponent>();
		for (auto ent : group) {
			const auto& [transform, quad] = instance->registry.get<TransformComponent, QuadComponent>(ent);
			if (quad.texture) {
				Renderer2D::drawTexturedQuad(transform.global, quad.texture->base, rendercontext);
			}
			else {
				Renderer2D::drawColoredQuad(transform.global, quad.color, rendercontext);
			}

		}

		return true;
	}

	bool32 pushToRenderer3D(Renderer3D::Context* rendercontext, Scene::Instance* instance) {

		auto group = instance->registry.view<TransformComponent, MeshComponent>();

		for (auto ent : group) {
			const auto& [transform, mesh] = instance->registry.get<TransformComponent, MeshComponent>(ent);

			if (mesh.mesh) {
				Renderer3D::drawMeshSafe(transform.global, mesh.mesh, mesh.materials.getArray(), rendercontext);
			}

		}
		return true;
	}
#define RENDER_3D 1

	//renders a scene to the currently bound renderpass.
	bool32 Scene::render(Renderer2D::Context* renderer2d, Renderer3D::Context* renderer3d, Instance* instance, const Scene::Camera& camera) {

		//apply the projection correction to the projection matrix;
		glm::mat4 projectioncorrection = glm::transpose(glm::mat4(
			glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 0.5f, 0.5f),
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
		));

		//update the scenebuffer with the camera & lights
		SceneBufferData* scenebufferdata;
		GFX::mapBuffer((void**)&scenebufferdata, instance->scenebuffer);
		*scenebufferdata = SceneBufferData{};

		scenebufferdata->projection = projectioncorrection * camera.projection;
		scenebufferdata->view = camera.view;

		//begin a pass to draw the cubemap
		Engine::Renderer3D::begin(renderer3d);
		Engine::Renderer2D::begin(renderer2d);

		Platform::GFX::DescriptorSet globaldescriptorsets[] = { Engine::Assets::Scene::getSceneDescriptorSet(instance) };
		//draw the skybox
		if (instance->skybox) {
			if (instance->skyboxshader->Ready()) {
				Engine::Renderer3D::drawCube(glm::mat4(1.0f), instance->skyboxshader, renderer3d);
				Engine::Renderer3D::flush(renderer3d, globaldescriptorsets[0]);
			}
		}
	
		auto view = instance->registry.view<LightComponent, TransformComponent>();

		//in the future we should find the lights that are closest to the camera or some kind of algorithm
		uint32 index = 0;
		for (auto ent : view) {
			const auto& [lc, tc] = instance->registry.get<LightComponent, TransformComponent>(ent);

			if (lc.type == LightType::POINT) {
				glm::vec4 pos = tc.global * glm::vec4(0.0f, 0.0f, 0.0f, 1.0);
				if (index >= MAX_LIGHTS) {
					continue;
				}

				scenebufferdata->lights[index].position = pos;
				scenebufferdata->lights[index].tint = glm::vec4(lc.tint, 1.0f);
				scenebufferdata->lights[index].brightness = lc.brightness;
				scenebufferdata->lights[index].linearfallof = lc.linear;
				scenebufferdata->lights[index].quadraticfallof = lc.quadratic;

				index++;
				Renderer3D::drawFlatColoredCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(tc.global[3])), glm::vec3(0.1f)), glm::vec4(lc.tint, 1.0f), renderer3d);
			}

			//might want to add the option to have multiple directional lights
			if (lc.type == LightType::DIRECTIONAL) {
				scenebufferdata->skylight.direction = tc.global[1];
				scenebufferdata->skylight.tint = glm::vec4(lc.tint, 0.0f);

			}
		}
		scenebufferdata->lightcount = index;
	
		//push the renderables into the renderer
		Scene::pushToRenderer2D(renderer2d, instance);
		Renderer2D::flush(renderer2d, { globaldescriptorsets, 1 });

#if RENDER_3D 

		pushToRenderer3D(renderer3d, instance);
		Renderer3D::flush(renderer3d, globaldescriptorsets[0]);

		Renderer3D::end(renderer3d);
		Renderer2D::end(renderer2d);

#endif
		return true;
	}

	bool32 Scene::updateComponents(Scene::Instance* instance, real32 displayaspectratio) {

		auto cameraentities = instance->registry.view<CameraComponent>();

		for (entt::entity cameraentity : cameraentities) {
			auto& camera = instance->registry.get<CameraComponent>(cameraentity);

			//real32 displayaspectratio = (real32)context->windowwidth / (real32)context->windowheight;
			
			if (camera.orthographic) {

				//calculate correct values for left and right
				
				real32 left = displayaspectratio * -1.0f * camera.zoom;
				real32 right = displayaspectratio * 1.0f * camera.zoom;
				real32 top = -1.0f * camera.zoom;
				real32 bottom = 1.0f * camera.zoom;

				camera.projection = glm::ortho(left, right, bottom, top);
			}
		}
		
		return true;
	}


	PH::Platform::GFX::DescriptorSetLayout Scene::getSceneDescriptorSetLayout(Scene::Instance* instance) {
		return Scene::descriptorsetlayout;
	}

	PH::Platform::GFX::DescriptorSet Scene::getSceneDescriptorSet(Instance* instance) {
		return instance->scenedescriptor;
	}

	bool32 PH_ENGINE_API Scene::isEntityValid(EntityHandle handle, Instance* instance) {
		return instance->registry.valid((entt::entity)handle);
	}

	bool32 serializeTag(YAML::Emitter& out, const TagComponent& tag) {

		out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap; // tagcomponent
		out << YAML::Key << "Tag" << YAML::Value << tag.tag.getC_Str();
		out << YAML::Key << "UUID" << YAML::Value << tag.uuid;
		out << YAML::EndMap; //tagcomponent

		return true;
	}

	bool32 serializeTransform(YAML::Emitter& out, const TransformComponent& transform) {

		out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap; //transformcomp
		
		out << YAML::Key << "global";
		out << YAML::Value << YAML::BeginSeq;
		glm::mat4 transpose = glm::transpose(transform.global);
		out << transpose[0];
		out << transpose[1];
		out << transpose[2];
		out << transpose[3];
		out << YAML::EndSeq;

		out << YAML::Key << "local";
		out << YAML::Value << YAML::BeginSeq;
		transpose = glm::transpose(transform.local);
		out << transpose[0];
		out << transpose[1];
		out << transpose[2];
		out << transpose[3];
		out << YAML::EndSeq;

		out << YAML::EndMap; //transformcomp

		return true;
	}

	bool32 serializeQuadComponent(YAML::Emitter& out, const QuadComponent& quad) {
		out << YAML::Key << "QuadComponent" << YAML::Value << YAML::BeginMap; //quadcomponent
		out << YAML::Key << "Color" << YAML::Value << quad.color;
		if (quad.texture) {
			out << YAML::Key << "Texture" << YAML::Value << quad.texture->id;
		}
		out << YAML::EndMap; //quadcomponent
		return true;
	}

	bool32 serializeMeshComponent(YAML::Emitter& out, const MeshComponent& mesh) {
		
		out << YAML::Key << "MeshComponent" << YAML::Value << YAML::BeginMap; //quadcomponent

		if (mesh.mesh) {
			out << YAML::Key << "Mesh" << YAML::Value << mesh.mesh->id;
		}
		out << YAML::Key << "Materials" << YAML::Value << YAML::BeginSeq;//materials
		
		for (auto mat : mesh.materials) {
			if (mat) {
				out << mat->id;
			}
			else {
				out << 0;
			}
		}
		out << YAML::EndSeq;

		out << YAML::EndMap; //quadcomponent
		return true;
	}

	bool32 serializeLightComponent(YAML::Emitter& out, const LightComponent& light) {

		out << YAML::Key << "LightComponent" << YAML::Value << YAML::BeginMap; //quadcomponent

		out << YAML::Key << "Tint" << YAML::Value << light.tint;
		out << YAML::Key << "Brightness" << YAML::Value << light.brightness;
		out << YAML::Key << "Linear" << YAML::Value << light.linear;
		out << YAML::Key << "Quadratic" << YAML::Value << light.quadratic;

		if (light.type == LightType::POINT) {
			out << YAML::Key << "Type" << YAML::Value << "POINT";
		}

		if (light.type == LightType::DIRECTIONAL) {
			out << YAML::Key << "Type" << YAML::Value << "DIRECTIONAL";
		}

		out << YAML::EndMap; //quadcomponent
		return true;

	}

	TransformComponent deserializeTransformComponent(YAML::Node node) {

		TransformComponent result{ glm::mat4(1.0f), glm::mat4(1.0f) };

		auto global = node["global"];
		if (global) { result.global = global.as<glm::mat4>(); }
		auto local = node["local"];
		if (local) { result.local = local.as<glm::mat4>(); }

		return result;
	}

	QuadComponent deserializeQuadComponent(YAML::Node node, Assets::Library* lib) {

		QuadComponent result{glm::vec4(0.0f)};

		auto color = node["Color"];
		if (color) { result.color = color.as<glm::vec4>(); }

		auto texture = node["Texture"];
		if (texture) {
			UUID id = texture.as<UUID>();

			Engine::Assets::TextureImage* im = lib->get<Assets::TextureImage>(id);
			if (im) {
				result.texture = im;
				result.textureid = im->id;
			}

		}

		return result;
	}

	MeshComponent deserializeMeshComponent(YAML::Node node, Assets::Library* lib) {

		MeshComponent result{};
		const auto& mesh = node["Mesh"];
		if (mesh) { 
			UUID meshid = mesh.as<UUID>();
			result.mesh = lib->getLazyLoaded<Engine::Assets::Mesh>(meshid);
		}
		
		if (!result.mesh) {
			return result;
		}

		result.materials = Engine::DynamicArray<Engine::Assets::MaterialInstance*>::create(result.mesh->submeshcount);
		for (uint32 i = 0; i < result.materials.getCapacity(); i++) {
			result.materials[i] = nullptr;
		}

		const auto& materials = node["Materials"];
		if (materials) {
			uint32 index = 0;
			for (auto& mat : materials) {
				if (index >= result.materials.getCapacity()) {
					break;
				}

				UUID id = mat.as<UUID>();
				Engine::Assets::MaterialInstance* m = lib->getLazyLoaded<Assets::MaterialInstance>(id);

				if (m) {
					result.materials[index] = m;
				}
				else {
					WARN << "Failed to find material: " << id << "for mesh: " << result.mesh->references[0].getC_Str() << "\n";
					result.materials[index] = nullptr;
				}
				index++;
			}
		}
		return result;
	}

	LightComponent deserializeLightComponent(YAML::Node node, Assets::Library* lib) {

		LightComponent result{};
		if (node["Tint"]) {
			result.tint = node["Tint"].as<glm::vec3>();
		}
		auto n = node["Brightness"];
		if (n) {
			result.brightness = n.as<real32>();
		}
		n = node["Linear"];
		if (n) {
			result.linear = n.as<real32>();
		}
		n = node["Quadratic"];
		if (n) {
			result.quadratic = n.as<real32>();
		}
		n = node["Type"];
		if (n) {
			String typestr = n.as<String>();

			if (typestr == "POINT") {
				result.type = LightType::POINT;
			}
			if (typestr == "DIRECTIONAL") {
				result.type = LightType::DIRECTIONAL;
			}

			String::destroy(&typestr);
		}
		return result;
	}

	bool32 serializeScene(YAML::Emitter& out, Scene* scene) {

		out << YAML::BeginMap; //scene
		out << YAML::Key << "Scene" << YAML::Key << "unnamed";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq; // entities

		Scene::Instance* instance = scene->instance;

		auto entities = instance->registry.view<TagComponent>();
		for (auto entity : entities) {
			out << YAML::BeginMap; // entity

			const TagComponent& tag = instance->registry.get<TagComponent>(entity);
			serializeTag(out, tag);

			const TransformComponent* t = instance->registry.try_get<TransformComponent>(entity);
			if (t) { serializeTransform(out, *t); }

			const QuadComponent* q = instance->registry.try_get<QuadComponent>(entity);
			if (q) { serializeQuadComponent(out, *q); }

			const MeshComponent* m = instance->registry.try_get<MeshComponent>(entity);
			if (m) { serializeMeshComponent(out, *m); }

			const LightComponent* l = instance->registry.try_get<LightComponent>(entity);
			if (l) { serializeLightComponent(out, *l); }

			out << YAML::EndMap; // entity
		}

		out << YAML::EndSeq; //enities
		out << YAML::EndMap; //scene

		return true;
	}

	bool32 deserializeScene(YAML::Node& root, Library* lib, Scene* scene) {

		YAML::Node& data = root;
		if (!data["Scene"]) {
			WARN << "scene file is not a valid scene!\n";
			return false;
		}

		Scene::Instance* result = Scene::createInstance();

		auto entities = data["Entities"];
		for (auto entity : entities) {

			auto tag = entity["TagComponent"];
			if (!tag) {
				WARN << "found entity without tag component\n";
				continue;
			}

			uint64 uuid = tag["UUID"].as<uint64>();
			std::string tagstr = tag["Tag"].as<std::string>();
			Scene::Entity sceneentity = Scene::createEntity(result, tagstr.c_str(), uuid);

			//add the transform component
			auto tc = entity["TransformComponent"];
			if (tc) {
				*sceneentity.getComponent<TransformComponent>() = deserializeTransformComponent(tc);
			}

			//add the quad component
			auto qc = entity["QuadComponent"];
			if (qc) {
				sceneentity.addComponent<QuadComponent>(deserializeQuadComponent(qc, lib));
			}

			//add the quad component
			auto mc = entity["MeshComponent"];
			if (mc) {
				sceneentity.addComponent<MeshComponent>(deserializeMeshComponent(mc, lib));
			}

			//add the light component
			auto lc = entity["LightComponent"];
			if (lc) {
				sceneentity.addComponent<LightComponent>(deserializeLightComponent(lc, lib));
			}
		}
		scene->instance = result;
		return true;
	}
}