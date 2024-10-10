#pragma once
#include "Common/shader_gl.hpp"
#include "Common/pipeline_gl.hpp"
#include "Common/shader_set_gl.hpp"

enum class EShaderType : UInt8;
template<typename API>
struct Model;
template<typename API>
struct Mesh;
template<typename API>
struct Material;
template<typename API>
struct Texture;
template<typename GraphicsAPI>
class Simulation;

class OpenGL
{
public:
    using Image = UInt32;
    using Buffer = UInt32;
    using Buffer = UInt32;
    using Shader = ShaderGL;
    using Pipeline = PipelineGL;
    using ShaderSet = ShaderSetGL;

private:
    DynamicArray<Buffer> buffers;
    DynamicArray<Buffer> arrays;
    DynamicArray<Image> images;

    DynamicArray<Shader> shaders;
    HashMap<String, Handle<Shader>> shadersNameMap;
    DynamicArray<Pipeline> pipelines;
    DynamicArray<ShaderSet> shaderSets;

public:
    Void startup(Simulation<OpenGL>& simulation);

    Void draw_model(Simulation<OpenGL>& simulation, const Model<OpenGL>& model);
    Void draw_quad();

    Handle<Shader> create_shader(const String& filePath, EShaderType type);
    Handle<Shader> create_shader(const String& shaderCode, const String& shaderName, EShaderType type);

    Handle<Pipeline> create_pipeline(const ShaderSet& shaderSet);
    Handle<ShaderSet> create_shader_set(const ShaderSet& shaderSet);

    Void create_model_render_data(Simulation<OpenGL>& simulation, Model<OpenGL>& model);
    Void create_mesh_buffers(Mesh<OpenGL>& mesh);
    Void create_material_images(Simulation<OpenGL>& simulation, Material<OpenGL>& material);
    Void create_texture_image(Texture<OpenGL>& texture);

    [[nodiscard]]
    const Handle<Shader>& get_shader_handle(const String& name)  const;
    Shader& get_shader(const String& name);
    Shader& get_shader(const Handle<Shader> handle);
    Pipeline& get_pipeline(const Handle<Pipeline> handle);
    Pipeline& get_pipeline(const Handle<ShaderSet> handle);
    ShaderSet& get_shader_set(const Handle<ShaderSet> handle);

    Image& get_image(const Handle<Image> handle);
    Buffer& get_buffer(const Handle<Buffer> handle);
    Buffer& get_array(const Handle<Buffer> handle);

    Void shutdown();

private:
    static Void gl_debug(UInt32 source,
                         UInt32 type,
                         UInt32 id,
                         UInt32 severity,
                         Int32  length,
                         const Char* message,
                         const Void* userParam);
};