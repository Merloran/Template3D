#define __STDC_LIB_EXT1__ 1
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#include "resource_manager.hpp"

#include "Common/model.hpp"
#include "Common/material.hpp"
#include "Common/mesh.hpp"
#include "Common/texture.hpp"

#include <filesystem>

#include "Common/color.hpp"

Void ResourceManager::startup()
{
	SPDLOG_INFO("Resource Manager startup.");
	Model defaultModel{};
	defaultModel.name = "DefaultModel";

	Mesh defaultMesh{};
	defaultMesh.name = "DefaultMesh";

	// UV MAP        Y
	//	    ____     + X
	//     | TT |
	//     |    |
	//	   | BB |
	// ____|    |____
	//| LL   BT   RR |
	//|____      ____|
	//     | FF |
	//     |____|

	defaultMesh.vertexes =
	{
		// Front face
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f / 3.0f, 0.25f } }, // Left-Bottom
		{ {  1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 2.0f / 3.0f, 0.25f } }, // Right-Bottom
		{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 2.0f / 3.0f,  0.0f } }, // Right-Top
		{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f / 3.0f,  0.0f } }, // Left-Top

	    // Back face
		{ {  1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 2.0f / 3.0f,  0.5f } }, // Right-Bottom
		{ { -1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f / 3.0f,  0.5f } }, // Left-Bottom
		{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f / 3.0f, 0.75f } }, // Left-Top
		{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 2.0f / 3.0f, 0.75f } }, // Right-Top

	    // Left face
		{ { -1.0f, -1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f / 3.0f,  0.5f } }, // Back-Bottom
		{ { -1.0f, -1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f / 3.0f, 0.25f } }, // Front-Bottom
		{ { -1.0f,  1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, {        0.0f, 0.25f } }, // Front-Top
		{ { -1.0f,  1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, {        0.0f,  0.5f } }, // Back-Top

	    // Right face
		{ {  1.0f, -1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 2.0f / 3.0f, 0.25f } }, // Front-Bottom
		{ {  1.0f, -1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 2.0f / 3.0f,  0.5f } }, // Back-Bottom
		{ {  1.0f,  1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, {        1.0f,  0.5f } }, // Back-Top
		{ {  1.0f,  1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, {        1.0f, 0.25f } }, // Front-Top

	    // Top face
		{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f / 3.0f,  1.0f } }, // Front-Left
		{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 2.0f / 3.0f,  1.0f } }, // Front-Right
		{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 2.0f / 3.0f, 0.75f } }, // Back-Right
		{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f / 3.0f, 0.75f } }, // Back-Left

	    // Bottom face
		{ { -1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f / 3.0f,  0.5f } }, // Back-Left
		{ {  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 2.0f / 3.0f,  0.5f } }, // Back-Right
		{ {  1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 2.0f / 3.0f, 0.25f } }, // Top-Right
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f / 3.0f, 0.25f } }, // Top-Left
	};

	defaultMesh.indexes =
	{
		 0,  1,  2,  2,  3,  1, // Front
		 4,  5,  6,  6,  7,  5, // Back
		 8,  9, 10, 10, 11,  9, // Left
		12, 13, 14, 14, 15, 13, // Right
		16, 17, 18, 18, 19, 17, // Top
		20, 21, 22, 22, 23, 21, // Bottom
	};

	defaultModel.meshes.push_back(create_mesh(defaultMesh));

	Material defaultMaterial{};
	defaultMaterial.name = "DefaultMaterial";

	defaultMaterial[ETextureType::Albedo] = create_texture({ 32, 32 },
		                                                   Color::GRAY,
		                                                   ETextureType::Albedo,
									  				       "DefaultBaseColor");

	defaultMaterial[ETextureType::Normal] = create_texture({ 32, 32 },
		                                                   Color::INDIGO, 
		                                                   ETextureType::Normal,
									  				       "DefaultNormal");

	defaultMaterial[ETextureType::Roughness] = create_texture({ 32, 32 },
															  Color::BLACK, 
															  ETextureType::Roughness,
									  						  "DefaultRoughness");

	defaultMaterial[ETextureType::Metalness] = create_texture({ 32, 32 },
															  Color::WHITE, 
															  ETextureType::Metalness,
									  						  "DefaultMetalness");

	defaultMaterial[ETextureType::AmbientOcclusion] = create_texture({ 32, 32 },
																     Color::BLACK, 
																     ETextureType::AmbientOcclusion,
																     "DefaultAmbientOcclusion");

	defaultModel.materials.push_back(create_material(defaultMaterial));

	create_model(defaultModel);
}

ResourceManager& ResourceManager::get()
{
	static ResourceManager instance;
	return instance;
}

Void ResourceManager::load_gltf_asset(const String &filePath)
{
	tinygltf::Model gltfModel;
	String error;
	String warning;

	tinygltf::TinyGLTF loader;

	if (!loader.LoadASCIIFromFile(&gltfModel, &error, &warning, filePath) || !warning.empty() || !error.empty())
	{
		SPDLOG_ERROR("Failed to load gltf file: {} - {} - {}", filePath, error, warning);
		return;
	}

	for (tinygltf::Node& gltfNode : gltfModel.nodes)
	{
		//TODO: change it later
		if (gltfNode.mesh == -1) //Skip nodes without meshes
		{
			continue;
		}
		load_model(filePath, gltfNode, gltfModel);
	}
}

Handle<Model> ResourceManager::load_model(const String &filePath, const tinygltf::Node &gltfNode, tinygltf::Model &gltfModel)
{
	std::filesystem::path assetPath(filePath);
	const String modelName = assetPath.stem().string() + gltfNode.name;
	if (nameToIdModels.contains(modelName))
	{
		SPDLOG_WARN("Model with name {} already exist!", modelName);
		//TODO: think of it
		return get_model_handle_by_name(modelName);
	}
	tinygltf::Mesh& gltfMesh = gltfModel.meshes[gltfNode.mesh];

	const UInt64 modelId = models.size();
	Model& model		 = models.emplace_back();

	model.meshes.reserve(gltfMesh.primitives.size());
	model.directory = assetPath.string();

	for (UInt64 i = 0; i < gltfMesh.primitives.size(); i++)
	{
		tinygltf::Primitive& primitive = gltfMesh.primitives[i];
		String meshName = modelName + std::to_string(i);
		Handle<Mesh> mesh = load_mesh(meshName, primitive, gltfModel);
		model.meshes.push_back(mesh);
		Handle<Material> material = get_material_handle_by_name("DefaultMaterial");
		if (primitive.material >= 0)
		{
			material = load_material(assetPath.parent_path().string(),
									 gltfModel.materials[primitive.material],
									 gltfModel);
		}
		model.materials.push_back(material);
	}

	const Handle<Model> modelHandle{ modelId };
	nameToIdModels[modelName] = modelHandle;
	model.name = modelName;

	return modelHandle;
}

Handle<Mesh> ResourceManager::load_mesh(const String& meshName, tinygltf::Primitive& primitive, tinygltf::Model& gltfModel)
{
	if (nameToIdMeshes.contains(meshName))
	{
		SPDLOG_WARN("Mesh with name {} already exist!", meshName);
		//TODO: think of it
		return get_mesh_handle_by_name(meshName);
	}

	const UInt64 meshId = meshes.size();
	Mesh& mesh			= meshes.emplace_back();

	const tinygltf::Accessor& indexesAccessor = gltfModel.accessors[primitive.indices];
	Int32 indexesType = indexesAccessor.componentType;
	Int32 indexesTypeCount = indexesAccessor.type;

	// Load indexes
	if (indexesTypeCount != TINYGLTF_TYPE_SCALAR)
	{
		SPDLOG_ERROR("Mesh indexes not loaded, not supported type: GLTF_TYPE {}; Name {}", indexesTypeCount, meshName);
		meshes.pop_back();
		return Handle<Mesh>::NONE;
	}
	
	switch (indexesType)
	{
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
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		{
			process_accessor<UInt32>(gltfModel, indexesAccessor, mesh.indexes);
			break;
		}
		default:
		{
			SPDLOG_ERROR("Mesh indexes not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", indexesType, meshName);
			meshes.pop_back();
			return Handle<Mesh>::NONE;
		}
	}

	// Load positions
	const tinygltf::Accessor& positionsAccessor = gltfModel.accessors[primitive.attributes["POSITION"]];
	Int32 positionsType = positionsAccessor.componentType;
	Int32 positionsTypeCount = positionsAccessor.type;

	if (positionsTypeCount != TINYGLTF_TYPE_VEC3)
	{
		SPDLOG_ERROR("Mesh positions not loaded, not supported type: GLTF_TYPE {}; Name {}", positionsTypeCount, meshName);
		meshes.pop_back();
		return Handle<Mesh>::NONE;
	}

	if (positionsType != TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		SPDLOG_ERROR("Mesh positions not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", positionsType, meshName);
		meshes.pop_back();
		return Handle<Mesh>::NONE;
	}

	process_accessor<FVector3>(gltfModel, positionsAccessor, mesh.vertexes, offsetof(Vertex, position));

	const Int32 bufferViewId = positionsAccessor.bufferView;

	const tinygltf::BufferView& bufferView = gltfModel.bufferViews[bufferViewId];
	tinygltf::Buffer& bufferData = gltfModel.buffers[bufferView.buffer];
	UInt8* dataBegin = bufferData.data.data() + positionsAccessor.byteOffset + bufferView.byteOffset;

	UInt64 stride = bufferView.byteStride;
	if (stride == 0)
	{
		stride = sizeof(FVector3);
	}

	mesh.vertexes.resize(positionsAccessor.count);

	for (UInt64 i = 0; i < positionsAccessor.count; ++i)
	{
		mesh.vertexes[i].position = *reinterpret_cast<FVector3*>(dataBegin + stride * i);
	}

	// Load normals
	const tinygltf::Accessor& normalsAccessor = gltfModel.accessors[primitive.attributes["NORMAL"]];
	Int32 normalsType = normalsAccessor.componentType;
	Int32 normalsTypeCount = normalsAccessor.type;

	if (normalsTypeCount != TINYGLTF_TYPE_VEC3)
	{
		SPDLOG_ERROR("Mesh normals not loaded, not supported type: GLTF_TYPE {}; Name {}", normalsTypeCount, meshName);
		meshes.pop_back();
		return Handle<Mesh>::NONE;
	}

	if (normalsType != TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		SPDLOG_ERROR("Mesh normals not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", normalsType, meshName);
		meshes.pop_back();
		return Handle<Mesh>::NONE;
	}

	process_accessor<FVector3>(gltfModel, positionsAccessor, mesh.vertexes, offsetof(Vertex, normal));

	// Load uvs
	const tinygltf::Accessor& uvsAccessor = gltfModel.accessors[primitive.attributes["TEXCOORD_0"]];
	Int32 uvsType = uvsAccessor.componentType;
	Int32 uvsTypeCount = uvsAccessor.type;

	if (uvsTypeCount != TINYGLTF_TYPE_VEC2)
	{
		SPDLOG_ERROR("Mesh uvs not loaded, not supported type: GLTF_TYPE {}; Name {}", uvsTypeCount, meshName);
		meshes.pop_back();
		return Handle<Mesh>::NONE;
	}

	if (uvsType != TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		SPDLOG_ERROR("Mesh uvs not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", uvsType, meshName);
		meshes.pop_back();
		return Handle<Mesh>::NONE;
	}

	process_accessor<FVector2>(gltfModel, positionsAccessor, mesh.vertexes, offsetof(Vertex, uv));

	const Handle<Mesh> meshHandle{ meshId };
	nameToIdMeshes[meshName] = meshHandle;
	mesh.name = meshName;

	return meshHandle;
}

Handle<Material> ResourceManager::load_material(const String& filePath, tinygltf::Material& gltfMaterial, const tinygltf::Model& gltfModel)
{
	if (nameToIdMaterials.contains(gltfMaterial.name))
	{
		SPDLOG_WARN("Material with name {} already exist!", gltfMaterial.name);
		//TODO: think of it
		return get_material_handle_by_name(gltfMaterial.name);
	}

	std::filesystem::path assetPath(filePath);
	const UInt64 materialId = materials.size();
	Material& material		= materials.emplace_back();

	const Int32 albedoId			  = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
	const Int32 metallicRoughnessId   = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
	const Int32 normalId			  = gltfMaterial.normalTexture.index;
	const Int32 ambientOcclusionId    = gltfMaterial.occlusionTexture.index;
	const Int32 emissionId		      = gltfMaterial.emissiveTexture.index;

	if (albedoId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[albedoId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		const std::filesystem::path textureName = image.uri;
		material[ETextureType::Albedo] = load_texture((assetPath / image.uri).string(),
													  textureName.stem().string(), 
													  ETextureType::Albedo);
	}

	if (metallicRoughnessId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[metallicRoughnessId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		const std::filesystem::path textureName = image.uri;
		material[ETextureType::RM] = load_texture((assetPath / image.uri).string(), 
												  textureName.stem().string(), 
												  ETextureType::RM);
	}

	if (normalId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[normalId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		const std::filesystem::path textureName = image.uri;
		material[ETextureType::Normal] = load_texture((assetPath / image.uri).string(),
													  textureName.stem().string(), 
													  ETextureType::Normal);
	}

	if (ambientOcclusionId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[ambientOcclusionId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		const std::filesystem::path textureName = image.uri;
		material[ETextureType::AmbientOcclusion] = load_texture((assetPath / image.uri).string(),
													            textureName.stem().string(), 
													            ETextureType::AmbientOcclusion);
	}

	if (emissionId >= 0)
	{
		const tinygltf::Texture& texture = gltfModel.textures[emissionId];
		const tinygltf::Image& image = gltfModel.images[texture.source];
		const std::filesystem::path textureName = image.uri;
		material[ETextureType::Emission] = load_texture((assetPath / image.uri).string(),
													    textureName.stem().string(), 
													    ETextureType::Emission);
	}

	auto iterator = gltfMaterial.extensions.find("KHR_materials_ior");
	if (iterator != gltfMaterial.extensions.end())
	{
		const tinygltf::Value &indexOfRefractionValue = iterator->second.Get("ior");
		if (indexOfRefractionValue.IsNumber())
		{
			material.indexOfRefraction = Float32(indexOfRefractionValue.GetNumberAsDouble());
		}
	}

	const Handle<Material> materialHandle{ materialId };
	nameToIdMaterials[gltfMaterial.name] = materialHandle;
	material.name = gltfMaterial.name;

	return materialHandle;
}

Handle<Texture> ResourceManager::load_texture(const String& filePath, const String& textureName, const ETextureType type)
{
	if (nameToIdTextures.contains(textureName))
	{
		SPDLOG_WARN("Texture with name {} already exist!", textureName);
		//TODO: think of it
		return get_texture_handle_by_name(textureName);
	}

	
	const UInt64 textureId = textures.size();
	Texture& texture	   = textures.emplace_back();

	if (type == ETextureType::HDR)
	{
		texture.data = reinterpret_cast<UInt8*>(stbi_loadf(filePath.c_str(), 
														   &texture.size.x, 
														   &texture.size.y, 
														   &texture.channels, 
														   0));
	} else {
		texture.data = stbi_load(filePath.c_str(), 
								 &texture.size.x, 
								 &texture.size.y, 
								 &texture.channels, 
								 0);
	}

	texture.type = type;

	if (!texture.data)
	{
		SPDLOG_ERROR("Texture {} loading failed.", filePath);
		textures.pop_back();
		return Handle<Texture>::NONE;
	}

	const Handle<Texture> textureHandle{ textureId };
	nameToIdTextures[textureName] = textureHandle;
	texture.name = textureName;

	return textureHandle;
}

Void ResourceManager::save_texture(const Texture& texture)
{
	stbi_flip_vertically_on_write(true);
	const Int32 result = stbi_write_png(texture.name.c_str(),
										texture.size.x,
										texture.size.y,
										texture.channels,
										texture.data,
										texture.size.x * texture.channels);

	if (result == 0)
	{
		SPDLOG_ERROR("Failed to save texture: {}", texture.name);
	}
}

Handle<Model> ResourceManager::create_model(const Model& model)
{
	const UInt64 modelId = meshes.size();
	const Handle<Model> modelHandle{ modelId };
	nameToIdModels[model.name] = modelHandle;
	models.push_back(model);
	return modelHandle;
}

Handle<Mesh> ResourceManager::create_mesh(const Mesh& mesh)
{
	const UInt64 meshId = meshes.size();
	const Handle<Mesh> meshHandle{ meshId };
	nameToIdMeshes[mesh.name] = meshHandle;
	meshes.push_back(mesh);
	return meshHandle;
}

Handle<Material> ResourceManager::create_material(const Material& material)
{
	const UInt64 materialId = materials.size();
	materials.push_back(material);
	const Handle<Material> materialHandle{ materialId };
	nameToIdMaterials[material.name] = materialHandle;
	return materialHandle;
}

Handle<Texture> ResourceManager::create_texture(const Texture& texture)
{
	const UInt64 textureId = textures.size();
	const Handle<Texture> textureHandle{ textureId };
	nameToIdTextures[texture.name] = textureHandle;
	textures.push_back(texture);
	return textureHandle;
}

Handle<Texture> ResourceManager::create_texture(const UVector2& size, const Color& fillColor, ETextureType type, const String& name)
{
	const UInt64 textureId = textures.size();
	const Handle<Texture> textureHandle{ textureId };
	nameToIdTextures[name] = textureHandle;
	Texture& texture = textures.emplace_back();
	texture.size = size;
	texture.channels = 4;
	texture.type = type;
    const UInt64 bytesCount = UInt64(texture.channels) * size.x * size.y;
	texture.data = static_cast<UInt8*>(malloc(bytesCount));
	for (UInt64 i = 0; i < bytesCount; ++i)
	{
		texture.data[i] = fillColor[i & (4 - 1)]; // Faster modulo 4
	}
	texture.name = name;
	return textureHandle;
}

Model& ResourceManager::get_model_by_name(const String& name)
{
	const auto& iterator = nameToIdModels.find(name);
	if (iterator == nameToIdModels.end() || iterator->second.id >= models.size())
	{
		SPDLOG_WARN("Model {} not found, returned default.", name);
		return models[0];
	}

	return models[iterator->second.id];
}

Model& ResourceManager::get_model_by_handle(const Handle<Model> handle)
{
	if (handle.id >= models.size())
	{
		SPDLOG_WARN("Model {} not found, returned default.", handle.id);
		return models[0];
	}
	return models[handle.id];
}

Mesh& ResourceManager::get_mesh_by_name(const String& name)
{
	const auto& iterator = nameToIdMeshes.find(name);
	if (iterator == nameToIdMeshes.end() || iterator->second.id >= meshes.size())
	{
		SPDLOG_WARN("Mesh {} not found, returned default.", name);
		return meshes[0];
	}

	return meshes[iterator->second.id];
}

Mesh& ResourceManager::get_mesh_by_handle(const Handle<Mesh> handle)
{
	if (handle.id >= meshes.size())
	{
		SPDLOG_WARN("Mesh {} not found, returned default.", handle.id);
		return meshes[0];
	}
	return meshes[handle.id];
}

Material& ResourceManager::get_material_by_name(const String& name)
{
	const auto& iterator = nameToIdMaterials.find(name);
	if (iterator == nameToIdMaterials.end() || iterator->second.id >= materials.size())
	{
		SPDLOG_WARN("Material {} not found, returned default.", name);
		return materials[0];
	}

	return materials[iterator->second.id];
}

Material& ResourceManager::get_material_by_handle(const Handle<Material> handle)
{
	if (handle.id >= materials.size())
	{
		SPDLOG_WARN("Material {} not found, returned default.", handle.id);
		return materials[0];
	}
	return materials[handle.id];
}

Material& ResourceManager::get_default_material()
{
	return get_material_by_name("DefaultMaterial");
}

Texture& ResourceManager::get_texture_by_name(const String& name)
{
	const auto& iterator = nameToIdTextures.find(name);
	if (iterator == nameToIdTextures.end() || iterator->second.id >= textures.size())
	{
		SPDLOG_WARN("Texture {} not found, returned default.", name);
		return textures[0];
	}

	return textures[iterator->second.id];
}

Texture& ResourceManager::get_texture_by_handle(const Handle<Texture> handle)
{
	if (handle.id >= textures.size())
	{
		SPDLOG_WARN("Texture {} not found, returned default.", handle.id);
		return textures[0];
	}
	return textures[handle.id];
}

const Handle<Model>& ResourceManager::get_model_handle_by_name(const String& name) const
{
	const auto& iterator = nameToIdModels.find(name);
	if (iterator == nameToIdModels.end())
	{
		SPDLOG_WARN("Model handle {} not found, returned none.", name);
		return Handle<Model>::NONE;
	}
	return iterator->second;
}

const Handle<Mesh>& ResourceManager::get_mesh_handle_by_name(const String& name) const
{
	const auto& iterator = nameToIdMeshes.find(name);
	if (iterator == nameToIdMeshes.end())
	{
		SPDLOG_WARN("Mesh handle {} not found, returned none.", name);
		return Handle<Mesh>::NONE;
	}
	return iterator->second;
}

const Handle<Material>& ResourceManager::get_material_handle_by_name(const String& name) const
{
	const auto& iterator = nameToIdMaterials.find(name);
	if (iterator == nameToIdMaterials.end())
	{
		SPDLOG_WARN("Material handle {} not found, returned none.", name);
		return get_material_handle_by_name("DefaultMaterial");
	}
	return iterator->second;
}

const Handle<Texture>& ResourceManager::get_texture_handle_by_name(const String& name) const
{
	const auto& iterator = nameToIdTextures.find(name);
	if (iterator == nameToIdTextures.end())
	{
		SPDLOG_WARN("Texture handle {} not found, returned none.", name);
		return Handle<Texture>::NONE;
	}
	return iterator->second;
}

const DynamicArray<Model>& ResourceManager::get_models() const
{
	return models;
}

const DynamicArray<Mesh>& ResourceManager::get_meshes() const
{
	return meshes;
}

const DynamicArray<Material>& ResourceManager::get_materials() const
{
	return materials;
}

const DynamicArray<Texture>& ResourceManager::get_textures() const
{
	return textures;
}

Void ResourceManager::shutdown()
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