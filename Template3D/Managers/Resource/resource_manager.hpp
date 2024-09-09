#pragma once
#include <tiny_gltf.h>

#include "Common/vertex.hpp"

struct Color;

namespace std::filesystem
{
	class path;
}

struct Texture;
struct Material;
struct Model;
struct Mesh;
enum class ETextureType : Int16;

class ResourceManager
{
public:
	const String TEXTURES_PATH = "Resources/Textures/";
	const String ASSETS_PATH   = "Resources/Assets/";

private:
	HashMap<String, Handle<Model>> nameToIdModels;
	DynamicArray<Model> models;

	HashMap<String, Handle<Mesh>> nameToIdMeshes;
	DynamicArray<Mesh> meshes;

	HashMap<String, Handle<Material>> nameToIdMaterials;
	DynamicArray<Material> materials;

	HashMap<String, Handle<Texture>> nameToIdTextures;
	DynamicArray<Texture> textures;

public:
	ResourceManager(ResourceManager&) = delete;

	static ResourceManager& get();
	Void startup();

	Void load_gltf_asset(const String& filePath);

	Handle<Model>    load_model(const String &filePath,const tinygltf::Node &gltfNode, tinygltf::Model &gltfModel);
	Handle<Mesh>     load_mesh(const String &meshName, tinygltf::Primitive &primitive, tinygltf::Model &gltfModel);
	Handle<Material> load_material(const String &filePath, 
								   tinygltf::Material &gltfMaterial,
								   const tinygltf::Model &gltfModel);
	Handle<Texture>  load_texture(const String &filePath, const String &textureName, const ETextureType type);

    static Void save_texture(const Texture& texture);

	Handle<Model> create_model(const Model &model);
	Handle<Mesh> create_mesh(const Mesh& mesh);

	Handle<Material> create_material(const Material& material);
	Handle<Texture> create_texture(const Texture &texture);
	Handle<Texture> create_texture(const UVector2 &size, const Color& fillColor, ETextureType type, const String &name);

	Model    &get_model_by_name(const String &name);
	Model	 &get_model_by_handle(const Handle<Model> handle);
	Mesh     &get_mesh_by_name(const String &name);
	Mesh     &get_mesh_by_handle(const Handle<Mesh> handle);
	Material &get_material_by_name(const String &name);
	Material &get_material_by_handle(const Handle<Material> handle);
	Material &get_default_material();
	Texture  &get_texture_by_name(const String &name);
	Texture  &get_texture_by_handle(const Handle<Texture> handle);

	[[nodiscard]]
	const Handle<Model>		&get_model_handle_by_name(const String &name)	 const;
	[[nodiscard]]
	const Handle<Mesh>		&get_mesh_handle_by_name(const String &name)     const;
	[[nodiscard]]
	const Handle<Material>	&get_material_handle_by_name(const String &name) const;
	[[nodiscard]]
	const Handle<Texture>	&get_texture_handle_by_name(const String &name)  const;

	[[nodiscard]]
	const DynamicArray<Model>    &get_models()    const;
	[[nodiscard]]
	const DynamicArray<Mesh>     &get_meshes()    const;
	[[nodiscard]]
	const DynamicArray<Material> &get_materials() const;
	[[nodiscard]]
	const DynamicArray<Texture>  &get_textures()  const;

	Void shutdown();

private:
	ResourceManager() = default;
	~ResourceManager() = default;

	template<typename DataType, typename ArrayType>
    static Void process_accessor(tinygltf::Model& gltfModel, 
                                 const tinygltf::Accessor& accessor,
                                 DynamicArray<ArrayType>& outputData)
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

	template<typename DataType>
    static Void process_accessor(tinygltf::Model& gltfModel, 
                                 const tinygltf::Accessor& accessor, 
                                 DynamicArray<Vertex>& outputData, 
								 UInt64 offset)
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
};

