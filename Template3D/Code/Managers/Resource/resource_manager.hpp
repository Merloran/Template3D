#pragma once
#include "Common/vertex.hpp"
#include "Render/Common/graphics_api_concept.hpp"


struct Color;

namespace std::filesystem
{
    class path;
}

namespace tinygltf
{
    class Node;
    struct Primitive;
    class Model;
    struct Material;
    struct Accessor;
}

template<typename API>
struct Texture;
template<typename API>
struct Material;
template<typename API>
struct Model;
template<typename API>
struct Mesh;
enum class ETextureType : Int16;

template <GraphicsAPI API>
class ResourceManager
{
public:
    const String TEXTURES_PATH = "Resources/Textures/";
    const String ASSETS_PATH   = "Resources/Assets/";

private:
    HashMap<String, Handle<Model<API>>> modelsNameMap;
    DynamicArray<Model<API>> models;

    HashMap<String, Handle<Mesh<API>>> meshesNameMap;
    DynamicArray<Mesh<API>> meshes;

    HashMap<String, Handle<Material<API>>> materialsNameMap;
    DynamicArray<Material<API>> materials;

    HashMap<String, Handle<Texture<API>>> texturesNameMap;
    DynamicArray<Texture<API>> textures;

public:
    Void startup();

    Void load_gltf_asset(const String& filePath);

    Handle<Model<API>>    load_model(const String &filePath, const tinygltf::Node &gltfNode, tinygltf::Model &gltfModel);
    Handle<Mesh<API>>     load_mesh(const String &meshName, tinygltf::Primitive &primitive, tinygltf::Model &gltfModel);
    Handle<Material<API>> load_material(const String &filePath,
                                   tinygltf::Material &gltfMaterial,
                                   const tinygltf::Model &gltfModel);
    Handle<Texture<API>>  load_texture(const String &filePath, const String &textureName, const ETextureType type);

    static Void save_texture(const Texture<API>& texture);

    Handle<Model<API>> create_model(const Model<API>&model);
    Handle<Mesh<API>> create_mesh(const Mesh<API>& mesh);

    Handle<Material<API>> create_material(const Material<API>& material);
    Handle<Texture<API>> create_texture(const Texture<API>&texture);
    Handle<Texture<API>> create_texture(const UVector2 &size, const Color& fillColor, ETextureType type, const String &name);

    Model<API>    &get_model(const String &name);
    Model<API>    &get_model(const Handle<Model<API>> handle);
    Model<API>	  &get_default_model();
    Mesh<API>     &get_mesh(const String &name);
    Mesh<API>     &get_mesh(const Handle<Mesh<API>> handle);
    Material<API> &get_material(const String &name);
    Material<API> &get_material(const Handle<Material<API>> handle);
    Material<API> &get_default_material();
    Texture<API>  &get_texture(const String &name);
    Texture<API>  &get_texture(const Handle<Texture<API>> handle);

    [[nodiscard]]
    const Handle<Model<API>>    &get_model_handle(const String &name)	 const;
    [[nodiscard]]
    const Handle<Mesh<API>>     &get_mesh_handle(const String &name)     const;
    [[nodiscard]]
    const Handle<Material<API>>	&get_material_handle(const String &name) const;
    [[nodiscard]]
    const Handle<Texture<API>>  &get_texture_handle(const String &name)  const;

    [[nodiscard]]
    const DynamicArray<Model<API>>    &get_models()    const;
    [[nodiscard]]
    const DynamicArray<Mesh<API>>     &get_meshes()    const;
    [[nodiscard]]
    const DynamicArray<Material<API>> &get_materials() const;
    [[nodiscard]]
    const DynamicArray<Texture<API>>  &get_textures()  const;

    Void shutdown();

private:
    template<typename DataType, typename ArrayType>
    static Void process_accessor(tinygltf::Model& gltfModel,
                                 const tinygltf::Accessor& accessor,
                                 DynamicArray<ArrayType>& outputData);

    template<typename DataType>
    static Void process_accessor(tinygltf::Model& gltfModel,
                                 const tinygltf::Accessor& accessor,
                                 DynamicArray<Vertex>& outputData,
                                 UInt64 offset);
};



#include "resource_manager.inl"
