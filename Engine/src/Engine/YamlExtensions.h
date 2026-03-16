#pragma once

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <Platform/platformAPI.h>

namespace YAML {
	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {

			Node node;
			node.push_back(rhs[0]);
			node.push_back(rhs[1]);
			node.push_back(rhs[2]);
			node.push_back(rhs[3]);

			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}

			rhs[0] = node[0].as<float>();
			rhs[1] = node[1].as<float>();
			rhs[2] = node[2].as<float>();
			rhs[3] = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::mat4> {
		static Node encode(const glm::mat4& rhs) {

			Node node;
			node.push_back(rhs[0]);
			node.push_back(rhs[1]);
			node.push_back(rhs[2]);
			node.push_back(rhs[3]);
			return node;
		}

		static bool decode(const Node& node, glm::mat4& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}

			rhs[0] = node[0].as<glm::vec4>();
			rhs[1] = node[1].as<glm::vec4>();
			rhs[2] = node[2].as<glm::vec4>();
			rhs[3] = node[3].as<glm::vec4>();
			rhs = glm::transpose(rhs);

			return true;
		}
	};

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {

			Node node;
			node.push_back(rhs[0]);
			node.push_back(rhs[1]);
			node.push_back(rhs[2]);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) {
				return false;
			}

			rhs[0] = node[0].as<float>();
			rhs[1] = node[1].as<float>();
			rhs[2] = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<PH::Engine::String> {
		static Node encode(const PH::Engine::String& rhs) {

			Node node;
			node.push_back(rhs.getC_Str());
			return node;
		}

		static bool decode(const Node& node, PH::Engine::String& rhs) {
		
			std::string str = node.as<std::string>();
			rhs = PH::Engine::String::create(str.c_str());
			return true;
		}
	};

	template<>
	struct convert<PH::Engine::FilePath> {
		static Node encode(const PH::Engine::FilePath& rhs) {

			Node node;
			node.push_back(rhs.getC_str());
			return node;
		}

		static bool decode(const Node& node, PH::Engine::FilePath& rhs) {

			std::string str = node.as<std::string>();
			rhs = PH::Engine::FilePath::create(str.c_str());
			return true;
		}
	};

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec) {

		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;

		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec) {

		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;

		return out;
	}
}

namespace PH::Engine::FileIO {

	inline YAML::Node loadYamlfile(const char* path) {
		Platform::FileBuffer file;
		if (!Platform::loadFile(&file, path)) {
			WARN << "failed to load yaml file: " << path << "\n";
			return {};
		}
		
		YAML::Node result = YAML::Load((const char*)file.data); 
		Platform::unloadFile(&file);
		return result;
	}

	inline bool32 writeYamlFile(YAML::Emitter& out, const char* path) {
		Platform::FileBuffer file;
		file.data = (void*)out.c_str();
		file.size = out.size();

		return Platform::writeFile(file, path);
	}
}
