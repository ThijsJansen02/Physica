#pragma once

#include <Engine//Rendering.h>
#include <Engine/cppAPI/Rendering.hpp>
#include <stb/stb_truetype.h>

namespace PH::RpGui {

	extern Engine::Renderer2D::Wrapper renderer2D;
	//simple font struct, now only supports 96 characters, but it would be easy to expand this to support more characters by increasing the size of the bitmap and cdata arrays, and changing the parameters of the stbtt_BakeFontBitmap function call in the applicationInitialize function
	struct Font {
		Platform::GFX::Texture atlas;
		stbtt_bakedchar cdata_cpu[96];
		Platform::GFX::DescriptorSet cdata;
		Platform::GFX::Buffer cdatabuffer;


		uint32 bitmapwidth;
		real32 pixelheight;
	};

	Platform::GFX::DescriptorSetLayout createFontDescriptorSetLayout() {

		Platform::GFX::DescriptorBinding binding;
		binding.binding = 0;
		binding.descriptorcount = 1;
		binding.shaderstages = Platform::GFX::SHADER_STAGE_FRAGMENT_BIT;
		binding.type = Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Platform::GFX::DescriptorSetLayoutCreateinfo createinfo{};
		createinfo.bindings = { &binding, 1 };
		Platform::GFX::DescriptorSetLayout result;
		Platform::GFX::createDescriptorSetLayouts(&createinfo, &result, 1);
		return result;
	}

	Platform::GFX::DescriptorSetLayout fontuserlayout = PH_GFX_NULL;

	struct UVcoords {
		glm::vec2 bottomleft;
		glm::vec2 topright;
	};

	Font loadFont(const char* ttfpath, uint32 bitmapwidth, real32 pixelheight) {

		Font result = {};

		const uint32 charcount = 96;

		PH::Platform::FileBuffer ttf_buffer;
		PH::Platform::loadFile(&ttf_buffer, ttfpath);

		uint32 fontbitmapwidth = bitmapwidth;

		uint8* bitmap = (uint8*)Engine::Allocator::alloc(fontbitmapwidth * fontbitmapwidth);

		stbtt_BakeFontBitmap((const unsigned char*)ttf_buffer.data, 0, pixelheight, (unsigned char*)bitmap, fontbitmapwidth, fontbitmapwidth, 32, charcount, result.cdata_cpu); // no guarantee this fits!


		PH::Platform::unloadFile(&ttf_buffer);

		//create the texture for the font atlas
		PH::Platform::GFX::TextureCreateInfo textureinfo{};
		textureinfo.data = bitmap;
		textureinfo.format = PH::Platform::GFX::FORMAT_R8_SRGB;
		textureinfo.usage = PH::Platform::GFX::IMAGE_USAGE_SAMPLED_BIT;
		textureinfo.viewtype = PH::Platform::GFX::IMAGE_VIEW_TYPE_2D;
		textureinfo.ArrayLayers = 1;
		textureinfo.width = fontbitmapwidth;
		textureinfo.height = fontbitmapwidth;

		PH::Platform::GFX::createTextures(&textureinfo, &result.atlas, 1);

		if (fontuserlayout == PH_GFX_NULL) {
			fontuserlayout = createFontDescriptorSetLayout();
		}

		Engine::ArrayList<UVcoords> evs = Engine::ArrayList<UVcoords>::create(charcount);

		for (auto& c : result.cdata_cpu) {
			UVcoords coords{};
			coords.bottomleft = glm::vec2{ c.x0, c.y1 } / (real32)bitmapwidth;
			coords.topright = glm::vec2{ c.x1, c.y0 } / (real32)bitmapwidth;

			evs.pushBack(coords);
		}

		PH::Platform::GFX::DescriptorSetCreateinfo descriptorcreate{};
		descriptorcreate.dynamic = false;
		descriptorcreate.layout = fontuserlayout;

		PH::Platform::GFX::createDescriptorSets(&descriptorcreate, &result.cdata, 1);

		PH::Platform::GFX::BufferCreateinfo buffercreate{};
		buffercreate.bufferusage = PH::Platform::GFX::BufferUsageFlagBits::BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffercreate.data = evs.raw();
		buffercreate.dynamic = false;
		buffercreate.memoryproperties = Platform::GFX::MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		buffercreate.size = evs.getCount() * sizeof(UVcoords);

		PH::Platform::GFX::createBuffers(&buffercreate, &result.cdatabuffer, 1);

		//writing the font data into the set
		PH::Platform::GFX::DescriptorBufferInfo bufferinfo{};
		bufferinfo.buffer = result.cdatabuffer;
		bufferinfo.offset = 0;
		bufferinfo.range = evs.getCount() * sizeof(UVcoords);

		PH::Platform::GFX::DescriptorWrite write{};
		write.dstset = result.cdata;
		write.arrayelement = 0;
		write.descriptorcount = 1;
		write.dynamicwrite = false;
		write.dstbinding = 0;
		write.type = PH::Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorinfo = &bufferinfo;

		PH::Platform::GFX::updateDescriptorSets(&write, 1);

		evs.release();
		Engine::Allocator::dealloc(bitmap);

		result.bitmapwidth = bitmapwidth;
		result.pixelheight = pixelheight;

		return result;
	}

	void drawFromBottomLeft(glm::vec3 position, glm::vec2 size, glm::vec4 color, uint32 id) {
		glm::vec3 middle = glm::vec3(glm::vec2(position) + (size / 2.0f), position.z);
		RpGui::renderer2D.drawQuadWithID(
			middle,
			size,
			color,
			id
		);
	}

	void drawText(Font* font, const char* text, glm::vec2 position, real32 scale) {

		real32 x = 0.0f;
		real32 y = 0.0f;

		while (*text) {
			if (*text >= 32 && *text < 128) {
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(font->cdata_cpu, font->bitmapwidth, font->bitmapwidth, *text - 32, &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9

				glm::vec2 bottomleft = scale * glm::vec2{ q.x0, -q.y1 };
				glm::vec2 topright = scale * glm::vec2{ q.x1, -q.y0 };

				glm::vec2 size = topright - bottomleft;

				drawFromBottomLeft(
					glm::vec3(bottomleft + glm::vec2(position.x, position.y), 0.0f),
					size,
					glm::vec4(1.0f),
					*text - 32
				);
			}
			++text;
		}


	}
}