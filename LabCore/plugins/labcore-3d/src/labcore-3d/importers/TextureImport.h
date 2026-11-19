#pragma once

namespace PH::LabCore3D {

	PH::bool32 importTextureCallback(Engine::AssetLibrary* asset, const char* sourcepath);
	PH::bool32 onRenderTextureImportOptions(Engine::AssetLibrary* assets);
}
