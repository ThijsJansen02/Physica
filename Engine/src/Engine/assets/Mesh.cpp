#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Platform/platformAPI.h>
#include "../YamlExtensions.h"
#include "../AssetLibrary.h"
#include "Texture.h"


namespace PH::Engine::Assets {

	using namespace PH::Platform;

	Mesh::SubMesh processMesh(aiMesh* aimesh, const aiScene* aiscene, Mesh* mesh) {

		sizeptr offset = mesh->localib.getCount();
		sizeptr vertexoffset = mesh->localvb.getCount();

		for (unsigned int i = 0; i < aimesh->mNumVertices; i++)
		{
			Assets::Vertex vertex;
			// process vertex positions, normals and texture coordinates
			glm::vec3 vector;

			vector.x = aimesh->mVertices[i].x;
			vector.y = aimesh->mVertices[i].y;
			vector.z = aimesh->mVertices[i].z;
			vertex.position = vector;

			vector.x = aimesh->mNormals[i].x;
			vector.y = aimesh->mNormals[i].y;
			vector.z = aimesh->mNormals[i].z;
			vertex.normal = vector;

			if (aimesh->mTextureCoords[0]) { //if the mesh has texture coords
				glm::vec2 vec;
				vec.x = aimesh->mTextureCoords[0][i].x;
				vec.y = aimesh->mTextureCoords[0][i].y;
				vertex.UV = vec;
			}
			else {
				vertex.UV = glm::vec2(0.0f, 0.0f);
			}

			mesh->localvb.pushBack(vertex);
		}

		for (uint32 i = 0; i < aimesh->mNumFaces; i++)
		{
			aiFace face = aimesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				mesh->localib.pushBack(face.mIndices[j] + vertexoffset);
			}
		}

		Mesh::SubMesh result{};
		if (aimesh->mName.length > 0) {
			result.name = String::create(aimesh->mName.C_Str());
		}

		result.offset = offset;
		result.count = mesh->localib.getCount() - offset;
		return result;
	}

	bool32 processNodeImport(aiNode* node, const aiScene* aiscene, Mesh* mesh, Library* lib) {
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aimesh = aiscene->mMeshes[node->mMeshes[i]];
			if (aimesh->mPrimitiveTypes &= aiPrimitiveType_TRIANGLE) {
				Mesh::SubMesh submesh = processMesh(aimesh, aiscene, mesh);
				if (submesh.count > 0) {

					aiMaterial* meshmat = aiscene->mMaterials[aimesh->mMaterialIndex];
					auto materialname = Base::String<ArenaAllocator>::create(meshmat->GetName().C_Str());
					materialname.replace(':', '_');

					MaterialInstance* material = lib->getByReference<MaterialInstance>(materialname.getC_Str());

					mesh->defaultmaterials.pushBack(material->id);
					mesh->submeshes.pushBack(submesh);
				}
			}
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			if (!processNodeImport(node->mChildren[i], aiscene, mesh, lib)) {
				return false;
			}
		}
		return true;
	}

	bool32 processNode(aiNode* node, const aiScene* aiscene, Mesh* mesh) {

		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aimesh = aiscene->mMeshes[node->mMeshes[i]];
			if (aimesh->mPrimitiveTypes &= aiPrimitiveType_TRIANGLE) {
				Mesh::SubMesh submesh = processMesh(aimesh, aiscene, mesh);
				if (submesh.count > 0) {
					mesh->submeshes.pushBack(submesh);
				}
			}

		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			if (!processNode(node->mChildren[i], aiscene, mesh)) {
				return false;
			}
		}
		return true;
	}

	bool32 loadMesh(const aiScene* scene, Mesh* mesh) {


		mesh->localvb = ArrayList<Assets::Vertex>::create(1);
		mesh->localib = ArrayList<uint32>::create(1);
		mesh->submeshes = ArrayList<Mesh::SubMesh>::create(1);

		processNode(scene->mRootNode, scene, mesh);

		GFX::BufferCreateinfo createinfos[2];
		createinfos[0].bufferusage = GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT;
		createinfos[0].data = mesh->localvb.raw();
		createinfos[0].size = mesh->localvb.getCount() * sizeof(Assets::Vertex);
		createinfos[0].dynamic = false;
		createinfos[0].memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createinfos[1].bufferusage = GFX::BUFFER_USAGE_INDEX_BUFFER_BIT;
		createinfos[1].data = mesh->localib.raw();
		createinfos[1].size = mesh->localib.getCount() * sizeof(uint32);
		createinfos[1].dynamic = false;
		createinfos[1].memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;

		GFX::Buffer buffers[2];
		GFX::createBuffers(createinfos, buffers, 2);

		mesh->vertexbuffer = buffers[0];
		mesh->indexbuffer = buffers[1];
		mesh->submeshcount = mesh->submeshes.getCount();
		return true;
	}

	bool32 loadMeshImport(const aiScene* scene, Mesh* mesh, Library* lib) {

		mesh->localvb = ArrayList<Assets::Vertex>::create(1);
		mesh->localib = ArrayList<uint32>::create(1);
		mesh->submeshes = ArrayList<Mesh::SubMesh>::create(1);

		processNodeImport(scene->mRootNode, scene, mesh, lib);

		GFX::BufferCreateinfo createinfos[2];
		createinfos[0].bufferusage = GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT;
		createinfos[0].data = mesh->localvb.raw();
		createinfos[0].size = mesh->localvb.getCount() * sizeof(Assets::Vertex);
		createinfos[0].dynamic = false;
		createinfos[0].memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createinfos[1].bufferusage = GFX::BUFFER_USAGE_INDEX_BUFFER_BIT;
		createinfos[1].data = mesh->localib.raw();
		createinfos[1].size = mesh->localib.getCount() * sizeof(uint32);
		createinfos[1].dynamic = false;
		createinfos[1].memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;

		GFX::Buffer buffers[2];
		GFX::createBuffers(createinfos, buffers, 2);

		mesh->vertexbuffer = buffers[0];
		mesh->indexbuffer = buffers[1];
		mesh->submeshcount = mesh->submeshes.getCount();
		return true;
	}

	TextureImage* tryImportTexture(aiTextureType type, const aiMaterial* mat, const aiScene* scene, Library* lib) {

		for (unsigned int i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			TextureImage* im = lib->import<TextureImage>(str.C_Str());
			if (im) {
				return im;
			}
		}
		return nullptr;
	}

	//loads a mesh and creates materials for it base on the mastermaterial
	bool32 importMesh(const char* filepath, const char* targetdir, Mesh* mesh, Material* mastermaterial, Library* lib) {
		
		FileBuffer file;
		if (!Platform::loadFile(&file, filepath)) {
			Engine::WARN << "Failed to load file " << filepath << "\n";
			return false;
		}

		Assimp::Importer importer;

		//figure out the extension;
		Base::SubString pathsub = filepath;
		pathsub = pathsub.getFromLast('.');
		const aiScene* scene = importer.ReadFileFromMemory(file.data, file.size, aiProcess_SortByPType | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph, pathsub.getC_Str());
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Engine::WARN << "ERROR::ASSIMP::" << importer.GetErrorString() << "\n";
			return false;
		}

		//creating the new materials
		for (uint32 i = 0; i < scene->mNumMaterials; i++) {
			aiMaterial* mat = scene->mMaterials[i];

			//should make sure the lib always had a default texture
			auto* defaulttexture = lib->getLoadedByReference<TextureImage>("default_texture");

			MaterialInstance inst{};
			createMaterialInstance(mastermaterial, &inst, defaulttexture);

			String name = String::create(mat->GetName().C_Str());

			TextureImage* albedo = tryImportTexture(aiTextureType_BASE_COLOR, mat, scene, lib);
			setTextureForMaterialInstance(&inst, 1, 0, albedo);

			TextureImage* roughness = tryImportTexture(aiTextureType_DIFFUSE_ROUGHNESS, mat, scene, lib);
			setTextureForMaterialInstance(&inst, 2, 0, roughness);

			TextureImage* normal = tryImportTexture(aiTextureType_NORMALS, mat, scene, lib);
			setTextureForMaterialInstance(&inst, 3, 0, normal);

			String pathstring = String::create(targetdir);
			pathstring.replace('\\', '/');
			pathstring.append("/materials");

			name.replace(':', '_');

			lib->add<MaterialInstance>(name.getC_Str(), pathstring.getC_Str(), inst);
		}

		mesh->defaultmaterials = ArrayList<UUID>::create(scene->mNumMaterials);
		loadMeshImport(scene, mesh, lib);

		mesh->sourcepath = String::create(filepath);
		unloadFile(&file);
	}


	bool32 loadMeshsource(const char* filepath, Mesh* mesh) {

		FileBuffer file;
		if (!Platform::loadFile(&file, filepath)) {
			Engine::WARN << "Failed to load file " << filepath << "\n";
			return false;
		}

		Assimp::Importer importer;

		//figure out the extension;
		Base::SubString pathsub = filepath;
		pathsub = pathsub.getFromLast('.');
		const aiScene* scene = importer.ReadFileFromMemory(file.data, file.size, aiProcess_SortByPType | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph, pathsub.getC_Str());

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Engine::WARN << "ERROR::ASSIMP::" << importer.GetErrorString() << "\n";
			return false;
		}

		loadMesh(scene, mesh);

		mesh->sourcepath = String::create(filepath);
		unloadFile(&file);
	}

	Mesh createMeshFromData(Base::Array<Assets::Vertex> vertices, Base::Array<uint32> indices, bool32 copylocal) {

		Mesh result;

		GFX::BufferCreateinfo createinfos[2];
		createinfos[0].bufferusage = GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT;
		createinfos[0].data = vertices.data;
		createinfos[0].size = vertices.count * sizeof(Assets::Vertex);
		createinfos[0].dynamic = false;
		createinfos[0].memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;

		createinfos[1].bufferusage = GFX::BUFFER_USAGE_INDEX_BUFFER_BIT;
		createinfos[1].data = indices.data;
		createinfos[1].size = indices.count * sizeof(uint32);
		createinfos[1].dynamic = false;
		createinfos[1].memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;

		GFX::Buffer buffers[2];
		GFX::createBuffers(createinfos, buffers, 2);

		result.vertexbuffer = buffers[0];
		result.indexbuffer = buffers[1];
		if (copylocal) {
			result.localvb = Engine::ArrayList<Assets::Vertex>::create(vertices);
			result.localib = Engine::ArrayList<uint32>::create(indices);
		}
		
		Mesh::SubMesh submesh;
		submesh.count = indices.count;
		submesh.offset = 0;
		result.submeshes = Engine::ArrayList<Mesh::SubMesh>::create({&submesh, 1});
		result.submeshcount = 1;

		return result;
	}

	bool32 serializeMesh(YAML::Emitter& out, Mesh* mesh) {
		out << YAML::BeginMap;
		out << YAML::Key << "SourceFile" << YAML::Value << mesh->sourcepath.getC_Str();
		out << YAML::Key << "SubMeshCount" << YAML::Value << mesh->submeshcount;
		
		out << YAML::Key << "DefaultMaterials" << YAML::Value << YAML::BeginSeq;

		for (auto mat : mesh->defaultmaterials) {
			out << mat;
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;
		return true;
	}

	bool32 serializeMesh(const char* filepath, Mesh* mesh) {
		YAML::Emitter out;
		serializeMesh(out, mesh);
		FileIO::writeYamlFile(out, filepath);

		return true;
	}


	bool32 deserializeMesh(YAML::Node& root, Library* lib, Mesh* mesh, const char* path) {		

		auto sourcepath = root["SourceFile"];
		if (!sourcepath) {
			return false;
		}

		if (!loadMeshsource(sourcepath.as<String>().getC_Str(), mesh)) {
			Engine::WARN << "Failed to load mesh!\n";
			return false;
		}
		return true;
	}

	//runs when an asset is included in the libary
	bool32 Mesh::include(YAML::Node& root, Library* lib, Mesh* instance) {
		auto count = root["SubMeshCount"];
		if (count) {
			instance->submeshcount = count.as<uint32>();
		}

		instance->defaultmaterials = Engine::ArrayList<UUID>::create();
		auto defaultmaterials = root["DefaultMaterials"];
		if (defaultmaterials) {
			for (auto mat : defaultmaterials) {
				instance->defaultmaterials.pushBack(mat.as<UUID>());
			}
		}
		else {
			auto dm = lib->getByReference<MaterialInstance>("default_material");
			for (uint32 i = 0; i < instance->submeshcount; i++) {
				instance->defaultmaterials.pushBack(dm->id);
			}
		}
		return true;
	}
}