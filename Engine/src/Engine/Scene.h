#pragma once
#include "Engine.h"
#include "Engine/Rendering.h"
#include "Engine/Assets/Texture.h"
#include "Engine/Asset.h"

#include <entt/entt.hpp>

namespace PH::Engine::Assets {
	class Library;
}


/// <summary>
/// note on the weird API...
/// 
/// the scene environment was first envisioned as just a namespace with an instance class to hold the data of a specic instance... 
/// but then i came to the conclusion that scene should be an asset just as every other item that needs saving etc. so the whole namespace needed to become a struct
/// </summary>
namespace PH::Engine::Assets {

	struct Scene;

	bool32 serializeScene(YAML::Emitter& out, Scene* scene);
	bool32 deserializeScene(YAML::Node& root, Library* lib, Scene* Scene);
	
	PH_ENGINE_API struct Scene : public Asset {
	public:
		typedef uint32 EntityHandle;
	
	public: 

		struct Instance;
		Instance* instance;

		//the descriptorsetlayout of the scenedescriptorset scene, normally bound as set 1;
		volatile static Platform::GFX::DescriptorSetLayout descriptorsetlayout;


		/// <summary>
		/// a storage container type class
		/// </summary>
		struct Entity {
		
			template<typename component>
			component* getComponent() {
				return Scene::getComponent<component>(handle, parent);
			}

			template<typename component> 
			void addComponent(const component& C) {
				Scene::addComponent<component>(handle, C, parent);
			}

			template<typename component>
			bool32 removeComponent() {
				return Scene::removeComponent<component>(handle, parent);
			}

			template<typename component>
			bool32 hasComponent() {
				if (getComponent<component>()) {
					return true;
				}
				return false;
			}

			bool32 valid() {
				if (!parent) {
					return false;
				}
				return Scene::isEntityValid(handle, parent);
			}

			Instance* parent;
			EntityHandle handle;
		};

		//serializes the asset into the yaml emitter
		static void serialize(YAML::Emitter& out, Scene* instance) {
			serializeScene(out, instance);
			return;
		}

		//deserializes the asset, the path parameter is only there for debug messages...
		static bool32 deserialize(YAML::Node& root, Library* lib, Scene* result, const char* path) {
			return deserializeScene(root, lib, result);
		}

		static constexpr const char* getExtension() {
			return ".phscene";
		}

		static constexpr AssetType getType() {
			return AssetType::SCENE;
		}

		template <class T>
		struct entt_allocator {
			using value_type = T;
			entt_allocator() noexcept {};

			template <class U> entt_allocator(const entt_allocator<U>&) noexcept {

			}
			T* allocate(std::size_t n) {
				return (T*)Engine::Allocator::alloc(n * sizeof(T));
			}
			void deallocate(T* p, std::size_t n) {
				Engine::Allocator::dealloc((void*)p);
			}
		};

		struct Instance {
			entt::basic_registry<entt::entity, entt_allocator<entt::entity>> registry;

			//information for the scene ubo
			PH::Platform::GFX::Buffer scenebuffer;
			PH::Platform::GFX::DescriptorSet scenedescriptor;

			Engine::Assets::CubeMap* skybox;
			Engine::Assets::MaterialInstance* skyboxshader;

			String location;
		};

		struct Camera {
			glm::mat4 view;
			glm::mat4 projection;
		};


		template<class component>
		static bool32 addComponent(EntityHandle entity, const component& comp, Instance* instance) 
		{
			instance->registry.emplace<component>((entt::entity)entity, comp);
			return true;
		}

		template<class component>
		static component* getComponent(EntityHandle entity, Instance* instance) 
		{
			return instance->registry.try_get<component>((entt::entity)entity);
		}

		template<class component>
		static bool32 removeComponent(EntityHandle entity, Instance* instance) 
		{
			instance->registry.remove<component>((entt::entity)entity);
			return true;
		}

		static Entity PH_ENGINE_API createEntity(Instance* instance, const char* name, uint64 uuid);
		static bool32 PH_ENGINE_API isEntityValid(EntityHandle handle, Scene::Instance* instance);
		static bool32 PH_ENGINE_API destroyEntity(EntityHandle entity, Instance* instace);

		static Instance* createInstance();
		static bool32 setSkyBox(Assets::CubeMap* map, Assets::MaterialInstance* shader, Instance* instance);

		//might move this to engine because it has greater global appearence
		static PH::Platform::GFX::DescriptorSetLayout getSceneDescriptorSetLayout(Instance* instance);
		static PH::Platform::GFX::DescriptorSet getSceneDescriptorSet(Instance* instance);

		//renders a scene to the currently bound renderpass.
		static bool32 render(Renderer2D::Context* renderer2d, Renderer3D::Context* renderer3d, Scene::Instance* instance, const Scene::Camera& camera);
		static bool32 pushToRenderer2D(Renderer2D::Context* rendercontext, Scene::Instance* instance);

		//platform context should become something of the form display context
		static bool32 updateComponents(Scene::Instance* instance, real32 displayaspectratio);
	};

	template <class T, class U>
	inline constexpr bool operator==(const Scene::entt_allocator<T>& l, const Scene::entt_allocator<U>& r) noexcept {
		return sizeof(T) == sizeof(U);
	}

	template <class T, class U>
	inline constexpr bool operator!= (const Scene::entt_allocator<T>& l, const Scene::entt_allocator<U>& r) noexcept {
		return sizeof(T) != sizeof(U);
	}

	inline bool32 operator==(Scene::Entity left, Scene::Entity right) {
		return left.handle == right.handle && left.parent == right.parent;
	}

	inline Scene createScene() {
		Scene result;
		result.instance = Scene::createInstance();
		result.status = AssetStatus::LOADED;
		return result;
	}
}