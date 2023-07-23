#pragma once
#include <tiny_gltf.h>

template<typename Type>
struct Handle;


namespace std {
	namespace filesystem {
		class path;
	}
}

struct Texture;
struct Material;
struct Model;
struct Mesh;
enum class ETextureType : Int8;

class SResourceManager
{
public:
	const std::string TEXTURES_PATH = "Resources/Textures/";
	const std::string ASSETS_PATH	= "Resources/Assets/";

	SResourceManager(SResourceManager&) = delete;

	static SResourceManager& get();
	void startup();

	void load_gltf_asset(const std::string& filePath);
	void load_gltf_asset(const std::filesystem::path& filePath);

	Handle<Model>    load_model(const std::filesystem::path & filePath, tinygltf::Mesh& gltfMesh, tinygltf::Model& gltfModel);
	Handle<Mesh>     load_mesh(const std::string& meshName, tinygltf::Primitive& primitive, tinygltf::Model& gltfModel);
	Handle<Material> load_material(const std::filesystem::path& assetPath, tinygltf::Material& gltfMaterial, tinygltf::Model& gltfModel);
	Handle<Texture>  load_texture(const std::filesystem::path& filePath, const std::string& textureName, ETextureType type);

	Handle<Material> create_material(const Material& material, const std::string& name);

	Model    &get_model_by_name(const std::string& name);
	Mesh     &get_mesh_by_name(const std::string& name);
	Material &get_material_by_name(const std::string& name);
	Texture  &get_texture_by_name(const std::string& name);

	const std::vector<Model>    &get_models()    const;
	const std::vector<Mesh>     &get_meshes()    const;
	const std::vector<Material> &get_materials() const;
	const std::vector<Texture>  &get_textures()  const;

	void shutdown();

protected:
	template<typename DataType, typename ArrayType>
	void process_accessor(tinygltf::Model& gltfModel, const tinygltf::Accessor& accessor, std::vector<ArrayType>& outputData)
	{
		Int32 bufferViewId = accessor.bufferView;

		tinygltf::BufferView& bufferView = gltfModel.bufferViews[bufferViewId];
		tinygltf::Buffer& bufferData = (gltfModel.buffers[bufferView.buffer]);
		UInt8* dataBegin = bufferData.data.data() + accessor.byteOffset + bufferView.byteOffset;

		UInt64 stride = bufferView.byteStride;
		if (stride == 0)
		{
			stride = sizeof(DataType);
		}

		outputData.resize(accessor.count);

		for (Int32 i = 0; i < accessor.count; i++)
		{
			outputData[i] = static_cast<ArrayType>(*reinterpret_cast<DataType*>(dataBegin + stride * i));
		}
	}

private:
	SResourceManager() = default;
	~SResourceManager() = default;

	std::unordered_map<std::string, Handle<Model>> nameToIdModels;
	std::vector<Model> models;

	std::unordered_map<std::string, Handle<Mesh>> nameToIdMeshes;
	std::vector<Mesh> meshes;

	std::unordered_map<std::string, Handle<Material>> nameToIdMaterials;
	std::vector<Material> materials;

	std::unordered_map<std::string, Handle<Texture>> nameToIdTextures;
	std::vector<Texture> textures;
};

