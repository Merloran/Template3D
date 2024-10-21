#pragma once

#include "resource_manager.hpp"
#include <tiny_gltf.h>

#include "Common/model.hpp"
#include "Common/material.hpp"
#include "Common/mesh.hpp"
#include "Common/texture.hpp"
#include "Common/color.hpp"

#include <filesystem>

template <GraphicsAPI API>
Void ResourceManager<API>::startup()
{
    SPDLOG_INFO("Resource Manager startup.");
    Model<API> defaultModel{};
    defaultModel.name = "DefaultModel";

    Mesh<API> defaultMesh{};
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
         0,  1,  2,  2,  3,  0, // Front
         4,  5,  6,  6,  7,  4, // Back
         8,  9, 10, 10, 11,  8, // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20, // Bottom
    };

    defaultModel.meshes.push_back(create_mesh(defaultMesh));

    Material<API> defaultMaterial{};
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

template <GraphicsAPI API>
Void ResourceManager<API>::load_gltf_asset(const String &filePath)
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

template <GraphicsAPI API>
Handle<Model<API>> ResourceManager<API>::load_model(const String &filePath, const tinygltf::Node &gltfNode, tinygltf::Model &gltfModel)
{
    std::filesystem::path assetPath(filePath);
    const String modelName = assetPath.stem().string() + gltfNode.name;
    if (modelsNameMap.contains(modelName))
    {
        SPDLOG_WARN("Model with name {} already exist!", modelName);
        //TODO: think of it
        return get_model_handle(modelName);
    }
    tinygltf::Mesh& gltfMesh = gltfModel.meshes[gltfNode.mesh];

    const UInt64 modelId = models.size();
    Model<API>& model    = models.emplace_back();

    model.meshes.reserve(gltfMesh.primitives.size());
    model.directory = assetPath.string();

    for (UInt64 i = 0; i < gltfMesh.primitives.size(); i++)
    {
        tinygltf::Primitive& primitive = gltfMesh.primitives[i];
        String meshName = modelName + std::to_string(i);
        Handle<Mesh<API>> mesh = load_mesh(meshName, primitive, gltfModel);
        model.meshes.push_back(mesh);
        Handle<Material<API>> material = get_default_material();
        if (primitive.material >= 0)
        {
            material = load_material(assetPath.parent_path().string(),
                                     gltfModel.materials[primitive.material],
                                     gltfModel);
        }
        model.materials.push_back(material);
    }

    const Handle<Model<API>> modelHandle{ modelId };
    modelsNameMap[modelName] = modelHandle;
    model.name = modelName;

    return modelHandle;
}

template <GraphicsAPI API>
Handle<Mesh<API>> ResourceManager<API>::load_mesh(const String& meshName, tinygltf::Primitive& primitive, tinygltf::Model& gltfModel)
{
    if (meshesNameMap.contains(meshName))
    {
        SPDLOG_WARN("Mesh with name {} already exist!", meshName);
        //TODO: think of it
        return get_mesh_handle(meshName);
    }

    const UInt64 meshId = meshes.size();
    Mesh<API>& mesh			= meshes.emplace_back();

    const tinygltf::Accessor& indexesAccessor = gltfModel.accessors[primitive.indices];
    Int32 indexesType = indexesAccessor.componentType;
    Int32 indexesTypeCount = indexesAccessor.type;

    // Load indexes
    if (indexesTypeCount != TINYGLTF_TYPE_SCALAR)
    {
        SPDLOG_ERROR("Mesh indexes not loaded, not supported type: GLTF_TYPE {}; Name {}", indexesTypeCount, meshName);
        meshes.pop_back();
        return Handle<Mesh<API>>::NONE;
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
            return Handle<Mesh<API>>::NONE;
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
        return Handle<Mesh<API>>::NONE;
    }

    if (positionsType != TINYGLTF_COMPONENT_TYPE_FLOAT)
    {
        SPDLOG_ERROR("Mesh positions not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", positionsType, meshName);
        meshes.pop_back();
        return Handle<Mesh<API>>::NONE;
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
        return Handle<Mesh<API>>::NONE;
    }

    if (normalsType != TINYGLTF_COMPONENT_TYPE_FLOAT)
    {
        SPDLOG_ERROR("Mesh normals not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", normalsType, meshName);
        meshes.pop_back();
        return Handle<Mesh<API>>::NONE;
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
        return Handle<Mesh<API>>::NONE;
    }

    if (uvsType != TINYGLTF_COMPONENT_TYPE_FLOAT)
    {
        SPDLOG_ERROR("Mesh uvs not loaded, not supported type: GLTF_COMPONENT_TYPE {}; Name {}", uvsType, meshName);
        meshes.pop_back();
        return Handle<Mesh<API>>::NONE;
    }

    process_accessor<FVector2>(gltfModel, positionsAccessor, mesh.vertexes, offsetof(Vertex, uv));

    const Handle<Mesh<API>> meshHandle{ meshId };
    meshesNameMap[meshName] = meshHandle;
    mesh.name = meshName;

    return meshHandle;
}

template <GraphicsAPI API>
Handle<Material<API>> ResourceManager<API>::load_material(const String& filePath, tinygltf::Material& gltfMaterial, const tinygltf::Model& gltfModel)
{
    if (materialsNameMap.contains(gltfMaterial.name))
    {
        SPDLOG_WARN("Material with name {} already exist!", gltfMaterial.name);
        //TODO: think of it
        return get_material_handle(gltfMaterial.name);
    }

    std::filesystem::path assetPath(filePath);
    const UInt64 materialId = materials.size();
    Material<API>& material		= materials.emplace_back();

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

    const Handle<Material<API>> materialHandle{ materialId };
    materialsNameMap[gltfMaterial.name] = materialHandle;
    material.name = gltfMaterial.name;

    return materialHandle;
}

template <GraphicsAPI API>
Handle<Texture<API>> ResourceManager<API>::load_texture(const String& filePath, const String& textureName, const ETextureType type)
{
    if (texturesNameMap.contains(textureName))
    {
        SPDLOG_WARN("Texture with name {} already exist!", textureName);
        //TODO: think of it
        return get_texture_handle(textureName);
    }

    
    const UInt64 textureId = textures.size();
    Texture<API>& texture  = textures.emplace_back();

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
        return Handle<Texture<API>>::NONE;
    }

    const Handle<Texture<API>> textureHandle{ textureId };
    texturesNameMap[textureName] = textureHandle;
    texture.name = textureName;

    return textureHandle;
}

template <GraphicsAPI API>
Void ResourceManager<API>::save_texture(const Texture<API>& texture)
{
    // stbi_flip_vertically_on_write(true);
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

template <GraphicsAPI API>
Handle<Model<API>> ResourceManager<API>::create_model(const Model<API>& model)
{
    const UInt64 modelId = models.size();
    const Handle<Model<API>> modelHandle{ modelId };
    modelsNameMap[model.name] = modelHandle;
    models.push_back(model);
    return modelHandle;
}

template <GraphicsAPI API>
Handle<Mesh<API>> ResourceManager<API>::create_mesh(const Mesh<API>& mesh)
{
    const UInt64 meshId = meshes.size();
    const Handle<Mesh<API>> meshHandle{ meshId };
    meshesNameMap[mesh.name] = meshHandle;
    meshes.push_back(mesh);
    return meshHandle;
}

template <GraphicsAPI API>
Handle<Material<API>> ResourceManager<API>::create_material(const Material<API>& material)
{
    const UInt64 materialId = materials.size();
    materials.push_back(material);
    const Handle<Material<API>> materialHandle{ materialId };
    materialsNameMap[material.name] = materialHandle;
    return materialHandle;
}

template <GraphicsAPI API>
Handle<Texture<API>> ResourceManager<API>::create_texture(const Texture<API>& texture)
{
    const UInt64 textureId = textures.size();
    const Handle<Texture<API>> textureHandle{ textureId };
    texturesNameMap[texture.name] = textureHandle;
    textures.push_back(texture);
    return textureHandle;
}

template <GraphicsAPI API>
Handle<Texture<API>> ResourceManager<API>::create_texture(const UVector2& size, const Color& fillColor, ETextureType type, const String& name)
{
    const UInt64 textureId = textures.size();
    const Handle<Texture<API>> textureHandle{ textureId };
    texturesNameMap[name] = textureHandle;
    Texture<API>& texture = textures.emplace_back();
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

template <GraphicsAPI API>
Model<API>& ResourceManager<API>::get_model(const String& name)
{
    const auto& iterator = modelsNameMap.find(name);
    if (iterator == modelsNameMap.end() || iterator->second.id >= models.size())
    {
        SPDLOG_WARN("Model {} not found, returned default.", name);
        return models[0];
    }

    return models[iterator->second.id];
}

template <GraphicsAPI API>
Model<API>& ResourceManager<API>::get_model(const Handle<Model<API>> handle)
{
    if (handle.id >= models.size())
    {
        SPDLOG_WARN("Model {} not found, returned default.", handle.id);
        return models[0];
    }
    return models[handle.id];
}

template <GraphicsAPI API>
Model<API>& ResourceManager<API>::get_default_model()
{
    return get_model("DefaultModel");
}

template <GraphicsAPI API>
Mesh<API>& ResourceManager<API>::get_mesh(const String& name)
{
    const auto& iterator = meshesNameMap.find(name);
    if (iterator == meshesNameMap.end() || iterator->second.id >= meshes.size())
    {
        SPDLOG_WARN("Mesh {} not found, returned default.", name);
        return meshes[0];
    }

    return meshes[iterator->second.id];
}

template <GraphicsAPI API>
Mesh<API>& ResourceManager<API>::get_mesh(const Handle<Mesh<API>> handle)
{
    if (handle.id >= meshes.size())
    {
        SPDLOG_WARN("Mesh {} not found, returned default.", handle.id);
        return meshes[0];
    }
    return meshes[handle.id];
}

template <GraphicsAPI API>
Material<API>& ResourceManager<API>::get_material(const String& name)
{
    const auto& iterator = materialsNameMap.find(name);
    if (iterator == materialsNameMap.end() || iterator->second.id >= materials.size())
    {
        SPDLOG_WARN("Material {} not found, returned default.", name);
        return materials[0];
    }

    return materials[iterator->second.id];
}

template <GraphicsAPI API>
Material<API>& ResourceManager<API>::get_material(const Handle<Material<API>> handle)
{
    if (handle.id >= materials.size())
    {
        SPDLOG_WARN("Material {} not found, returned default.", handle.id);
        return materials[0];
    }
    return materials[handle.id];
}

template <GraphicsAPI API>
Material<API>& ResourceManager<API>::get_default_material()
{
    return get_material("DefaultMaterial");
}

template <GraphicsAPI API>
Texture<API>& ResourceManager<API>::get_texture(const String& name)
{
    const auto& iterator = texturesNameMap.find(name);
    if (iterator == texturesNameMap.end() || iterator->second.id >= textures.size())
    {
        SPDLOG_WARN("Texture {} not found, returned default.", name);
        return textures[0];
    }

    return textures[iterator->second.id];
}

template <GraphicsAPI API>
Texture<API>& ResourceManager<API>::get_texture(const Handle<Texture<API>> handle)
{
    if (handle.id >= textures.size())
    {
        SPDLOG_WARN("Texture {} not found, returned default.", handle.id);
        return textures[0];
    }
    return textures[handle.id];
}

template <GraphicsAPI API>
const Handle<Model<API>>& ResourceManager<API>::get_model_handle(const String& name) const
{
    const auto& iterator = modelsNameMap.find(name);
    if (iterator == modelsNameMap.end())
    {
        SPDLOG_WARN("Model handle {} not found, returned none.", name);
        return Handle<Model<API>>::NONE;
    }
    return iterator->second;
}

template <GraphicsAPI API>
const Handle<Mesh<API>>& ResourceManager<API>::get_mesh_handle(const String& name) const
{
    const auto& iterator = meshesNameMap.find(name);
    if (iterator == meshesNameMap.end())
    {
        SPDLOG_WARN("Mesh handle {} not found, returned none.", name);
        return Handle<Mesh<API>>::NONE;
    }
    return iterator->second;
}

template <GraphicsAPI API>
const Handle<Material<API>>& ResourceManager<API>::get_material_handle(const String& name) const
{
    const auto& iterator = materialsNameMap.find(name);
    if (iterator == materialsNameMap.end())
    {
        SPDLOG_WARN("Material handle {} not found, returned none.", name);
        return get_material_handle("DefaultMaterial");
    }
    return iterator->second;
}

template <GraphicsAPI API>
const Handle<Texture<API>>& ResourceManager<API>::get_texture_handle(const String& name) const
{
    const auto& iterator = texturesNameMap.find(name);
    if (iterator == texturesNameMap.end())
    {
        SPDLOG_WARN("Texture handle {} not found, returned none.", name);
        return Handle<Texture<API>>::NONE;
    }
    return iterator->second;
}

template <GraphicsAPI API>
const DynamicArray<Model<API>>& ResourceManager<API>::get_models() const
{
    return models;
}

template <GraphicsAPI API>
const DynamicArray<Mesh<API>>& ResourceManager<API>::get_meshes() const
{
    return meshes;
}

template <GraphicsAPI API>
const DynamicArray<Material<API>>& ResourceManager<API>::get_materials() const
{
    return materials;
}

template <GraphicsAPI API>
const DynamicArray<Texture<API>>& ResourceManager<API>::get_textures() const
{
    return textures;
}

template <GraphicsAPI API>
Void ResourceManager<API>::shutdown()
{
    SPDLOG_INFO("Resource Manager shutdown.");
    texturesNameMap.clear();
    for (Texture<API>& texture : textures)
    {
        free(texture.data);
    }
    textures.clear();

    materialsNameMap.clear();
    materials.clear();

    meshesNameMap.clear();
    meshes.clear();

    modelsNameMap.clear();
    models.clear();
}

template <GraphicsAPI API>
template <typename DataType, typename ArrayType>
Void ResourceManager<API>::process_accessor(tinygltf::Model& gltfModel, const tinygltf::Accessor& accessor, DynamicArray<ArrayType>& outputData)
{
    const Int32 bufferViewId = accessor.bufferView;

    const tinygltf::BufferView& bufferView = gltfModel.bufferViews[bufferViewId];
    tinygltf::Buffer& bufferData = gltfModel.buffers[bufferView.buffer];
    UInt8* dataBegin = bufferData.data.data() + accessor.byteOffset + bufferView.byteOffset;

    UInt64 stride = bufferView.byteStride;
    if (stride == 0)
    {
        stride = sizeof(DataType);
    }

    outputData.resize(accessor.count);

    for (UInt64 i = 0; i < accessor.count; i++)
    {
        outputData[i] = static_cast<ArrayType>(*reinterpret_cast<DataType*>(dataBegin + stride * i));
    }
}

template <GraphicsAPI API>
template <typename DataType>
Void ResourceManager<API>::process_accessor(tinygltf::Model& gltfModel, const tinygltf::Accessor& accessor, DynamicArray<Vertex>& outputData, UInt64 offset)
{
    const Int32 bufferViewId = accessor.bufferView;

    const tinygltf::BufferView& bufferView = gltfModel.bufferViews[bufferViewId];
    tinygltf::Buffer& bufferData = gltfModel.buffers[bufferView.buffer];
    UInt8* dataBegin = bufferData.data.data() + accessor.byteOffset + bufferView.byteOffset;

    UInt64 stride = bufferView.byteStride;
    if (stride == 0)
    {
        stride = sizeof(DataType);
    }

    outputData.resize(accessor.count);

    UInt8* targetField = reinterpret_cast<UInt8*>(outputData.data()) + offset;
    for (UInt64 i = 0; i < accessor.count; i++)
    {
        *reinterpret_cast<DataType*>(targetField + sizeof(Vertex) * i) = *reinterpret_cast<DataType*>(dataBegin + stride * i);
    }
}
