#define __STDC_LIB_EXT1__ 1
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#include "resource_manager.hpp"

#include "Common/handle.hpp"
#include "Common/model.hpp"
#include "Common/material.hpp"
#include "Common/mesh.hpp"
#include "Common/texture.hpp"

#include <filesystem>

void SResourceManager::startup()
{
	SPDLOG_INFO("Resource Manager startup.");
	Material defaultMaterial;
	defaultMaterial.albedo			 = load_texture(TEXTURES_PATH + "Default/Albedo.png", "DefaultBaseColor", ETextureType::Albedo);
	defaultMaterial.normal			 = load_texture(TEXTURES_PATH + "Default/Normal.png", "DefaultNormal", ETextureType::Normal);
	defaultMaterial.roughness		 = load_texture(TEXTURES_PATH + "Default/Roughness.png", "DefaultRoughness", ETextureType::Roughness);
	defaultMaterial.metalness		 = load_texture(TEXTURES_PATH + "Default/Metalness.png", "DefaultMetalness", ETextureType::Metalness);
	defaultMaterial.ambientOcclusion = load_texture(TEXTURES_PATH + "Default/AmbientOcclusion.png", "DefaultAmbientOcclusion", ETextureType::AmbientOcclusion);
	create_material(defaultMaterial, "DefaultMaterial");
}

SResourceManager& SResourceManager::get()
{
	static SResourceManager instance;
	return instance;
}

void SResourceManager::load_gltf_asset(const std::string& filePath)
{
	load_gltf_asset(std::filesystem::path(filePath));
}

void SResourceManager::load_gltf_asset(const std::filesystem::path & filePath)
{
	tinygltf::Model gltfModel;
	std::string error;
	std::string warning;

	tinygltf::TinyGLTF loader;

	if (!loader.LoadASCIIFromFile(&gltfModel, &error, &warning, filePath.string()) || !warning.empty() || !error.empty())
	{
		SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", filePath.string(), error, warning);
		return;
	}

	for (tinygltf::Mesh& gltfMesh : gltfModel.meshes)
	{
		load_model(filePath, gltfMesh, gltfModel);
	}
}

Handle<Model> SResourceManager::load_model(const std::filesystem::path & filePath, tinygltf::Mesh &gltfMesh, tinygltf::Model &gltfModel)
{
	if (nameToIdModels.find(gltfMesh.name) != nameToIdModels.end())
	{
		SPDLOG_ERROR("Model with name {} already exist!", gltfMesh.name);
		return Handle<Model>::sNone;
	}

	models.emplace_back();
	Int64 modelId = models.size() - 1;
	Model &model = models[modelId];

	model.meshes.reserve(gltfMesh.primitives.size());
	model.directory = filePath.string();

	for (Int32 i = 0; i < gltfMesh.primitives.size(); i++)
	{
		tinygltf::Primitive& primitive = gltfMesh.primitives[i];
		std::string meshName = gltfMesh.name + std::to_string(i);
		Handle<Mesh> mesh = load_mesh(meshName, primitive, gltfModel);
		model.meshes.push_back(mesh);
		Handle<Material> material = load_material(filePath.parent_path(), gltfModel.materials[primitive.material], gltfModel);
		model.materials.push_back(material);
	}

	Handle<Model> modelHandle{ modelId };
	nameToIdModels[gltfMesh.name] = modelHandle;

	return modelHandle;
}

Handle<Mesh> SResourceManager::load_mesh(const std::string& meshName, tinygltf::Primitive& primitive, tinygltf::Model& gltfModel)
{
	if (nameToIdMeshes.find(meshName) != nameToIdMeshes.end())
	{
		SPDLOG_ERROR("Mesh with name {} already exist!", meshName);
		return Handle<Mesh>::sNone;
	}

	meshes.emplace_back();
	Int64 meshId = meshes.size() - 1;
	Mesh &mesh   = meshes[meshes.size() - 1];

	const tinygltf::Accessor& indexesAccessor = gltfModel.accessors[primitive.indices];
	Int32 indexesType						  = indexesAccessor.componentType;
	Int32 indexesTypeCount					  = indexesAccessor.type;

	// Load indexes
	switch (indexesType) {
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
		{
			process_accessor<UInt16>(gltfModel, indexesAccessor, mesh.indexes);
			break;
		}
		case TINYGLTF_COMPONENT_TYPE_SHORT: 
		{
			process_accessor<Int16>(gltfModel, indexesAccessor, mesh.indexes);
			break;
		}
		default:
		{
			SPDLOG_ERROR("Mesh indexes not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", indexesType, meshName);
			meshes.pop_back();
			return Handle<Mesh>::sNone;
		}	
	}


	// Load positions
	const tinygltf::Accessor& positionsAccessor = gltfModel.accessors[primitive.attributes["POSITION"]];
	Int32 positionsType							= positionsAccessor.componentType;
	Int32 positionsTypeCount					= positionsAccessor.type;

	if (positionsTypeCount == TINYGLTF_TYPE_VEC3) 
	{
		if (positionsType == TINYGLTF_COMPONENT_TYPE_FLOAT) 
		{
			process_accessor<glm::vec3>(gltfModel, positionsAccessor, mesh.positions);
		} else {
			SPDLOG_ERROR("Mesh positions not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", positionsType, meshName);
			meshes.pop_back();
			return Handle<Mesh>::sNone;
		}
	} else {
		SPDLOG_ERROR("Mesh positions not loaded, not supported type: GLTF_TYPE {}; Name {}", positionsTypeCount, meshName);
		meshes.pop_back();
		return Handle<Mesh>::sNone;
	}


	// Load normals
	const tinygltf::Accessor& normalsAccessor = gltfModel.accessors[primitive.attributes["NORMAL"]];
	Int32 normalsType = normalsAccessor.componentType;
	Int32 normalsTypeCount = normalsAccessor.type;

	if (normalsTypeCount == TINYGLTF_TYPE_VEC3)
	{
		if (normalsType == TINYGLTF_COMPONENT_TYPE_FLOAT)
		{
			process_accessor<glm::vec3>(gltfModel, normalsAccessor, mesh.normals);
		} else {
			SPDLOG_ERROR("Mesh normals not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", normalsType, meshName);
			meshes.pop_back();
			return Handle<Mesh>::sNone;
		}
	} else {
		SPDLOG_ERROR("Mesh normals not loaded, not supported type: GLTF_TYPE {}; Name {}", normalsTypeCount, meshName);
		meshes.pop_back();
		return Handle<Mesh>::sNone;
	}


	// Load uvs
	const tinygltf::Accessor& uvsAccessor = gltfModel.accessors[primitive.attributes["TEXCOORD_0"]];
	Int32 uvsType						  = uvsAccessor.componentType;
	Int32 uvsTypeCount					  = uvsAccessor.type;

	if (uvsTypeCount == TINYGLTF_TYPE_VEC2)
	{
		if (uvsType == TINYGLTF_COMPONENT_TYPE_FLOAT)
		{
			process_accessor<glm::vec2>(gltfModel, uvsAccessor, mesh.uvs);
		} else {
			SPDLOG_ERROR("Mesh uvs not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", uvsType, meshName);
			meshes.pop_back();
			return Handle<Mesh>::sNone;
		}
	} else {
		SPDLOG_ERROR("Mesh uvs not loaded, not supported type: GLTF_TYPE {}; Name {}", uvsTypeCount, meshName);
		meshes.pop_back();
		return Handle<Mesh>::sNone;
	}

	Handle<Mesh> meshHandle{ meshId };
	nameToIdMeshes[meshName] = meshHandle;

	return meshHandle;
}

Handle<Material> SResourceManager::load_material(const std::filesystem::path& assetPath, tinygltf::Material& gltfMaterial, tinygltf::Model& gltfModel)
{
	if (nameToIdMaterials.find(gltfMaterial.name) != nameToIdMaterials.end())
	{
		SPDLOG_ERROR("Material with name {} already exist!", gltfMaterial.name);
		return Handle<Material>::sNone;
	}

	materials.emplace_back();
	Int64 materialId   = materials.size() - 1;
	Material& material = materials[materialId];

	Int32 albedoId			  = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
	Int32 metallicRoughnessId = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
	Int32 normalId			  = gltfMaterial.normalTexture.index;
	Int32 ambientOcclusionId  = gltfMaterial.occlusionTexture.index;
	Int32 emissionId		  = gltfMaterial.emissiveTexture.index;

	if (albedoId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[albedoId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		std::filesystem::path textureName = image.uri;
		material.albedo = load_texture(assetPath / image.uri, textureName.stem().string(), ETextureType::Albedo);
	}

	if (metallicRoughnessId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[metallicRoughnessId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		std::filesystem::path textureName = image.uri;
		material.metalness = load_texture(assetPath / image.uri, textureName.stem().string(), ETextureType::RM);
		material.roughness = material.metalness;
	}

	if (normalId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[normalId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		std::filesystem::path textureName = image.uri;
		material.normal = load_texture(assetPath / image.uri, textureName.stem().string(), ETextureType::Normal);
	}

	if (ambientOcclusionId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[ambientOcclusionId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		std::filesystem::path textureName = image.uri;
		material.ambientOcclusion = load_texture(assetPath / image.uri, textureName.stem().string(), ETextureType::AmbientOcclusion);
	}

	if (emissionId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[emissionId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		std::filesystem::path textureName = image.uri;
		material.emission = load_texture(assetPath / image.uri, textureName.stem().string(), ETextureType::Emission);
	}

	Handle<Material> materialHandle{ materialId };
	nameToIdMaterials[gltfMaterial.name] = materialHandle;

	return materialHandle;
}

Handle<Texture> SResourceManager::load_texture(const std::filesystem::path& filePath, const std::string& textureName, ETextureType type)
{
	if (nameToIdTextures.find(textureName) != nameToIdTextures.end())
	{
		SPDLOG_ERROR("Texture with name {} already exist!", textureName);
		return Handle<Texture>::sNone;
	}

	textures.emplace_back();
	Int64 textureId = textures.size() - 1;

	Texture& texture = textures[textureId];
	texture.data = stbi_load(filePath.string().c_str(), &texture.size.x, &texture.size.y, &texture.channels, 0);
	texture.type = type;
	if (!texture.data)
	{
		SPDLOG_ERROR("Texture {} loading failed.", filePath.string());
		textures.pop_back();
		return Handle<Texture>::sNone;
	}

	Handle<Texture> textureHandle{ textureId };
	nameToIdTextures[textureName] = textureHandle;

	return textureHandle;
}

Handle<Material> SResourceManager::create_material(const Material& material, const std::string& name)
{
	materials.emplace_back(material);
	Handle<Material> materialHandle{ materials.size() - 1 };
	nameToIdMaterials[name] = materialHandle;

	return materialHandle;
}

Model& SResourceManager::get_model_by_name(const std::string& name)
{
	const auto& iterator = nameToIdModels.find(name);
	if (iterator == nameToIdModels.end() || iterator->second.id < 0 || iterator->second.id >= models.size())
	{
		SPDLOG_WARN("Model {} not found, returned default.", name);
		return models[0];
	}

	return models[iterator->second.id];
}

Mesh& SResourceManager::get_mesh_by_name(const std::string& name)
{
	const auto& iterator = nameToIdMeshes.find(name);
	if (iterator == nameToIdMeshes.end() || iterator->second.id < 0 || iterator->second.id >= meshes.size())
	{
		SPDLOG_WARN("Mesh {} not found, returned default.", name);
		return meshes[0];
	}

	return meshes[iterator->second.id];
}

Material& SResourceManager::get_material_by_name(const std::string& name)
{
	const auto& iterator = nameToIdMaterials.find(name);
	if (iterator == nameToIdMaterials.end() || iterator->second.id < 0 || iterator->second.id >= materials.size())
	{
		SPDLOG_WARN("Material {} not found, returned default.", name);
		return materials[0];
	}

	return materials[iterator->second.id];
}

Texture& SResourceManager::get_texture_by_name(const std::string& name)
{
	const auto& iterator = nameToIdTextures.find(name);
	if (iterator == nameToIdTextures.end() || iterator->second.id < 0 || iterator->second.id >= textures.size())
	{
		SPDLOG_WARN("Texture {} not found, returned default.", name);
		return textures[0];
	}

	return textures[iterator->second.id];
}

const std::vector<Model>& SResourceManager::get_models() const
{
	return models;
}

const std::vector<Mesh>& SResourceManager::get_meshes() const
{
	return meshes;
}

const std::vector<Material>& SResourceManager::get_materials() const
{
	return materials;
}

const std::vector<Texture>& SResourceManager::get_textures() const
{
	return textures;
}

void SResourceManager::shutdown()
{
	SPDLOG_INFO("Resource Manager shutdown.");
	nameToIdTextures.clear();
	for (Texture& texture : textures)
	{
		stbi_image_free(texture.data);
	}
	textures.clear();

	nameToIdMaterials.clear();
	materials.clear();

	nameToIdMeshes.clear();
	meshes.clear();

	nameToIdModels.clear();
	models.clear();
}