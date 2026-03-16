#include "Texture.h"
#include "../AssetLibrary.h"

#include <stb/stb_image.h>
#include "../YamlExtensions.h"


namespace PH::Engine::Assets {

	using namespace PH::Platform;

	bool32 loadTextureSource(const char* filepath, TextureImage* image) {

		TextureImage result = *image;

		FileBuffer file;
		loadFile(&file, filepath);

		if (!file.data) {
			WARN << "failed to load file: \n\n" << filepath << "\n";
			return false;
		}

		int32 width;
		int32 height;
		int32 channels;
		stbi_uc* imagedata = stbi_load_from_memory((stbi_uc*)file.data, file.size, &width, &height, &channels, STBI_rgb_alpha);

		GFX::TextureCreateInfo create;
		create.data = imagedata;
		create.format = GFX::FORMAT_R8G8B8A8_SRGB;
		create.usage = GFX::IMAGE_USAGE_SAMPLED_BIT;
		create.viewtype = GFX::IMAGE_VIEW_TYPE_2D;
		create.ArrayLayers = 1;
		create.width = width;
		create.height = height;

		GFX::createTextures(&create, &result.base, 1);

		result.format = create.format;
		result.sourcepath = String::create(filepath);
		result.type = AssetType::TEXTURE_IMAGE;

		*image = result;

		unloadFile(&file);
		stbi_image_free(imagedata);

		return true;
	}

	bool32 loadCubeMapSource(const char** filepaths, CubeMap* map) {
		
		CubeMap result{};

		stbi_uc* imagedata[6];
		int32 width;
		int32 height;

		//stbi_set_flip_vertically_on_load(true);
		for (uint32 i = 0; i < 6; i++) {

			FileBuffer file;
			loadFile(&file, filepaths[i]);

			if (!file.data) {
				WARN << "failed to load file: \n\n" << filepaths[i] << "\n";
				return false;
			}

			int32 channels;

			imagedata[i] = stbi_load_from_memory((stbi_uc*)file.data, file.size, &width, &height, &channels, STBI_rgb_alpha);
		}
		//stbi_set_flip_vertically_on_load(false);

		sizeptr imagesize = 4 * width * height * sizeof(uint8);
		void* data = Engine::Allocator::alloc(imagesize * 6);

		for (uint32 i = 0; i < 6; i++) {
			Base::copyMemory(imagedata[i], ((uint8*)data) + i * imagesize, imagesize);
		}

		GFX::TextureCreateInfo create;
		create.data = data;
		create.format = GFX::FORMAT_R8G8B8A8_SRGB;
		create.usage = GFX::IMAGE_USAGE_SAMPLED_BIT;
		create.viewtype = GFX::IMAGE_VIEW_TYPE_CUBE;
		create.ArrayLayers = 6;
		create.width = width;
		create.height = height;

		GFX::createTextures(&create, &result.base, 1);

		result.id = 0;
		result.type = AssetType::TEXTURE_IMAGE;
		result.references = {};

		for (uint32 i = 0; i < 6; i++) {
			stbi_image_free(imagedata[i]);
		}

		Engine::Allocator::dealloc(data);

		*map = result;

		return true;
	}

	bool32 serializeTextureImage(YAML::Emitter& out, TextureImage* image) {

		out << YAML::BeginMap;

		//save the texture specific properties
		out << YAML::Key << "SourceFile" << YAML::Value << image->sourcepath.getC_Str();

		out << YAML::EndMap;
		return true;
	}

	bool32 deserializeTextureImage(YAML::Node& root, TextureImage* result, const char* path) {
		
		auto s = root["SourceFile"];
		if (!s) {
			WARN << "Texture (path: " << path << " ) does not have a valid sourcefile\n";
			return false;
		}

		String sourcepath = s.as<String>();

		if (!loadTextureSource(sourcepath.getC_Str(), result)) {
			Engine::WARN << "Failed to load texture!\n";

			//for now just return null because there is no way yet to couple a source to it...
			return false;
		}

		String::destroy(&sourcepath);
		return true;
	}

	bool32 deserializeTextureImage(const char* path, TextureImage* image) {
		auto root = FileIO::loadYamlfile(path);
		return deserializeTextureImage(root, image, path);
	}


}