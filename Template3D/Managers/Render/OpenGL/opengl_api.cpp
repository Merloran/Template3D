#include "opengl_api.hpp"

#include "../../Resource/Common/model.hpp"
#include "../../Resource/Common/mesh.hpp"
#include "../../Resource/Common/texture.hpp"
#include "../../Resource/Common/material.hpp"

#include "../Core/simulation.hpp"

#include <filesystem>
#include <magic_enum.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


Void OpenGL::startup(Simulation<OpenGL>& simulation)
{
    if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    {
        SPDLOG_ERROR("Failed to initialize OpenGL loader!");
        simulation.shutdown();
        return;
    }

    // Create default shaders
    {
        String vertCode = "#version 460 core											\n"
                          "layout(location = 0) in vec3 position;						\n"
                          "layout(location = 1) in vec3 normal;							\n"
                          "layout(location = 2) in vec2 uvs;							\n"
                          "																\n"
                          "uniform mat4 viewProjection;									\n"
                          "uniform mat4 model;											\n"
                          "																\n"
                          "out vec3 worldPosition;										\n"
                          "out vec3 worldNormal;										\n"
                          "out vec2 uvsFragment;										\n"
                          "																\n"
                          "void main()													\n"
                          "{															\n"
                          "    uvsFragment = uvs;										\n"
                          "    worldPosition = vec3(model * vec4(position, 1.0f));		\n"
                          "    worldNormal = mat3(transpose(inverse(model))) * normal;	\n"
                          "    gl_Position = viewProjection * vec4(worldPosition, 1.0f);\n"
                          "}															\n";

        String fragCode = "#version 460 core													\n"
                          "out vec4 color;														\n"
                          "																		\n"
                          "uniform sampler2D Albedo;											\n"
                          "																		\n"
                          "in vec3 worldNormal;  												\n"
                          "in vec3 worldPosition;  												\n"
                          "in vec2 uvsFragment;													\n"
                          "																		\n"
                          "void main()															\n"
                          "{																	\n"
                          "    const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);					\n"
                          "    const vec3 lightPosition = vec3(10.0f, 50.0f, 10.0f);			\n"
                          "    vec4 objectColor = texture(Albedo, uvsFragment);					\n"
                          "    if (objectColor.w < 0.1f)										\n"
                          "    {																\n"
                          "    	discard;														\n"
                          "    }																\n"
                          "    																	\n"
                          "    // ambient														\n"
                          "    float ambientStrength = 0.5f;									\n"
                          "    vec3 ambient = ambientStrength * lightColor;					 	\n"
                          "    																 	\n"
                          "    // diffuse 												     	\n"
                          "    vec3 normal = normalize(worldNormal);							\n"
                          "    vec3 lightDirection = normalize(lightPosition - worldPosition); 	\n"
                          "    float diff = max(dot(normal, lightDirection), 0.0f);			 	\n"
                          "    vec3 diffuse = diff * lightColor;								\n"
                          "        															 	\n"
                          "    color = vec4((ambient + diffuse) * objectColor.xyz, 1.0f);		\n" 
                          "}																	\n";

        ShaderSet defaultSet;
        defaultSet.shaderHandles.push_back(create_shader(vertCode, "Default", EShaderType::Vertex));
        defaultSet.shaderHandles.push_back(create_shader(fragCode, "Default", EShaderType::Fragment));
        create_pipeline(defaultSet);
        simulation.resourceManager.get_default_material().shaderSetHandle = create_shader_set(defaultSet);
        create_model_render_data(simulation, simulation.resourceManager.get_default_model());
    }

    Int32 flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Void OpenGL::draw_model(Simulation<OpenGL>& simulation, const Model<OpenGL>& model)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (Int32 i = 0; i < model.meshes.size(); i++)
    {
        Mesh<OpenGL>& mesh = simulation.resourceManager.get_mesh(model.meshes[i]);
        Material<OpenGL>& material = simulation.resourceManager.get_material(model.materials[i]);
        Buffer vao = get_array(mesh.vertexesHandle);
        Pipeline& pipeline = get_pipeline(material.shaderSetHandle);
        pipeline.bind();
        FMatrix4 modelMatrix = FMatrix4(1.0f);
        static Float32 rot = 0.0f;
        rot += 0.02f;
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rot), FVector3(0.0f, 1.0f, 0.0f));

        FMatrix4 projectionMatrix = glm::perspective(glm::radians(70.0f),
                                                     simulation.displayManager.get_aspect_ratio(), 
                                                     0.001f, 
                                                     5000.0f);
        FMatrix4 viewMatrix = glm::lookAt(FVector3{ 0.0f, 0.0f, -10.0f }, 
                                          FVector3{ 0.0f, 0.0f, 0.0f }, 
                                          FVector3{ 0.0f, 1.0f, 0.0f });
        pipeline.set_mat4("model", modelMatrix);
        pipeline.set_mat4("viewProjection", projectionMatrix * viewMatrix);
        for (Int32 j = 0; j < material.textures.size(); ++j)
        {
            Handle<Texture<OpenGL>> handle = material.textures[j];
            if (handle == Handle<Texture<OpenGL>>::NONE)
            {
                continue;
            }
    
            const Texture<OpenGL>& texture = simulation.resourceManager.get_texture(handle);
            Image image = get_image(texture.imageHandle);
            String type(magic_enum::enum_name(texture.type));
            if (texture.type == ETextureType::Albedo)
            {
                pipeline.set_int(type, j);
                glActiveTexture(GL_TEXTURE0 + j);
                glBindTexture(GL_TEXTURE_2D, image);
            }
        }
        
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, mesh.indexes.size(), GL_UNSIGNED_INT, 0);
    }
    
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

Void OpenGL::draw_quad()
{
    static UInt32 vao = 0;
    static UInt32 vbo;
    if (vao == 0)
    {
        constexpr Float32 POSITIONS[] =
        {
            -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
        };

        constexpr Float32 UVS[] =
        {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };

        // setup plane VAO
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(POSITIONS) + sizeof(UVS), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(POSITIONS), &POSITIONS[0]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(POSITIONS), sizeof(UVS), &UVS[0]);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float32), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(Float32), (void*)sizeof(POSITIONS));
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

Handle<OpenGL::Shader> OpenGL::create_shader(const String& filePath, EShaderType type)
{
    const UInt64 shaderId = shaders.size();
    Shader& shader = shaders.emplace_back();

    Bool hasBeenCreated = shader.create(filePath, type);
    const String& name = shader.get_name();
    if (!hasBeenCreated)
    {
        SPDLOG_WARN("Shader {} has failed to create.", name);
        shaders.pop_back();
        return Handle<Shader>::NONE;
    }

    const Handle<Shader> handle{ shaderId };
    auto iterator = shadersNameMap.find(name);
    if (iterator != shadersNameMap.end())
    {
        SPDLOG_WARN("Shader {} already exists.", name);
        shader.clear();
        shaders.pop_back();
        return iterator->second;
    }

    shadersNameMap[name] = handle;
    return handle;
}

Handle<OpenGL::Shader> OpenGL::create_shader(const String& shaderCode, const String& shaderName, EShaderType type)
{
    const UInt64 shaderId = shaders.size();
    Shader& shader = shaders.emplace_back();

    Bool hasBeenCreated = shader.create(shaderName, shaderCode, type);
    const String& name = shader.get_name();
    if (!hasBeenCreated)
    {
        SPDLOG_WARN("Shader {} has failed to create.", name);
        shaders.pop_back();
        return Handle<Shader>::NONE;
    }

    const Handle<Shader> handle{ shaderId };
    auto iterator = shadersNameMap.find(name);
    if (iterator != shadersNameMap.end())
    {
        SPDLOG_WARN("Shader {} already exists.", name);
        shader.clear();
        shaders.pop_back();
        return iterator->second;
    }

    shadersNameMap[name] = handle;
    return handle;
}

Handle<OpenGL::Pipeline> OpenGL::create_pipeline(ShaderSet& shaderSet)
{
    const UInt64 pipelineId = pipelines.size();
    Pipeline& pipeline = pipelines.emplace_back();
    DynamicArray<Shader> shaders;
    shaders.reserve(shaderSet.shaderHandles.size());
    for (const Handle<ShaderGL> handle : shaderSet.shaderHandles)
    {
        shaders.push_back(get_shader(handle));
    }

    Bool hasBeenCreated = pipeline.create_graphics_pipeline(shaders);
    if (!hasBeenCreated)
    {
        SPDLOG_WARN("Pipeline {} has failed to create.", pipelineId);
        pipeline.clear();
        pipelines.pop_back();
        return Handle<Pipeline>::NONE;
    }

    const Handle<Pipeline> handle{ pipelineId };
    shaderSet.pipelineHandle = handle;
    return handle;
}

Handle<OpenGL::ShaderSet> OpenGL::create_shader_set(const ShaderSet& shaderSet)
{
    const UInt64 shaderSetId = shaderSets.size();
    shaderSets.push_back(shaderSet);
    const Handle<ShaderSet> handle{ shaderSetId };
    return handle;
}

Void OpenGL::create_model_render_data(Simulation<OpenGL>& simulation, Model<OpenGL>& model)
{
    for (UInt64 i = 0; i < model.meshes.size(); ++i)
    {
        create_mesh_buffers(simulation.resourceManager.get_mesh(model.meshes[i]));
        create_material_images(simulation, simulation.resourceManager.get_material(model.materials[i]));
    }
}

Void OpenGL::create_mesh_buffers(Mesh<OpenGL>& mesh)
{
    mesh.vertexesHandle = { arrays.size() };
    Buffer& arrayBuffer = arrays.emplace_back();
    glGenVertexArrays(1, &arrayBuffer);
    glBindVertexArray(arrayBuffer);

    Buffer& vertexesBuffer = buffers.emplace_back();
    glGenBuffers(1, &vertexesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexesBuffer);

    mesh.indexesHandle = { buffers.size() };
    Buffer& indexesBuffer = buffers.emplace_back();
    glGenBuffers(1, &indexesBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexesBuffer);


    glBufferData(GL_ARRAY_BUFFER, mesh.vertexes.size() * sizeof(Vertex), mesh.vertexes.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexes.size() * sizeof(UInt32), mesh.indexes.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 
                          3, 
                          GL_FLOAT, 
                          GL_FALSE, 
                          sizeof(Vertex), 
                          reinterpret_cast<Void*>(offsetof(Vertex, position)));
    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 
                          3, 
                          GL_FLOAT,
                          GL_FALSE, 
                          sizeof(Vertex),
                          reinterpret_cast<Void*>(offsetof(Vertex, normal)));
    // Texture position attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 
                          2, 
                          GL_FLOAT, 
                          GL_FALSE, 
                          sizeof(Vertex), 
                          reinterpret_cast<Void*>(offsetof(Vertex, uv)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Void OpenGL::create_material_images(Simulation<OpenGL>& simulation, Material<OpenGL>& material)
{
    for (Handle<Texture<OpenGL>>& texture : material.textures)
    {
        if (texture != Handle<Texture<OpenGL>>::NONE)
        {
            create_texture_image(simulation.resourceManager.get_texture(texture));
        }
    }
}

Void OpenGL::create_texture_image(Texture<OpenGL>& texture)
{
    texture.imageHandle = { images.size() };
    Image& image = images.emplace_back();

    glGenTextures(1, &image);
    glBindTexture(GL_TEXTURE_2D, image);
    UInt32 internalFormat, format, type;

    switch (texture.channels)
    {
        case 3:
        {
            if (texture.type == ETextureType::HDR)
            {
                internalFormat = GL_RGB32F;
                type = GL_FLOAT;
            }
            else {
                internalFormat = GL_RGB;
                type = GL_UNSIGNED_BYTE;
            }
            format = GL_RGB;
            break;
        }
        case 4:
        {
            if (texture.type == ETextureType::HDR)
            {
                internalFormat = GL_RGBA32F;
                type = GL_FLOAT;
            }
            else {
                internalFormat = GL_RGBA;
                type = GL_UNSIGNED_BYTE;
            }
            format = GL_RGBA;
            break;
        }
        default:
        {
            SPDLOG_WARN("Not supported count of channels: {}", texture.channels);
            glDeleteTextures(1, &image);
            image = 0;
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 texture.size.x,
                 texture.size.y,
                 0,
                 format,
                 type,
                 texture.data);

    if (texture.type == ETextureType::HDR)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

const Handle<OpenGL::Shader>& OpenGL::get_shader_handle(const String& name) const
{
    const auto& iterator = shadersNameMap.find(name);
    if (iterator == shadersNameMap.end())
    {
        SPDLOG_WARN("Shader handle {} not found, returned None.", name);
        return Handle<Shader>::NONE;
    }

    return iterator->second;
}

OpenGL::Shader& OpenGL::get_shader(const String& name)
{
    const auto& iterator = shadersNameMap.find(name);
    if (iterator == shadersNameMap.end())
    {
        SPDLOG_WARN("Shader {} not found, returned default.", name);
        return shaders[0];
    }

    return shaders[iterator->second.id];
}

OpenGL::Shader& OpenGL::get_shader(const Handle<Shader> handle)
{
    if (handle.id >= shaders.size())
    {
        SPDLOG_WARN("Shader {} not found, returned default.", handle.id);
        return shaders[0];
    }
    return shaders[handle.id];
}

OpenGL::Pipeline& OpenGL::get_pipeline(const Handle<Pipeline> handle)
{
    if (handle.id >= buffers.size())
    {
        SPDLOG_ERROR("Pipeline {} not found, returned default.", handle.id);
        return pipelines[0];
    }
    return pipelines[handle.id];
}

OpenGL::Pipeline& OpenGL::get_pipeline(const Handle<ShaderSet> handle)
{
    ShaderSet& shaderSet = get_shader_set(handle);
    if (shaderSet.pipelineHandle.id >= buffers.size())
    {
        SPDLOG_ERROR("Pipeline {} not found, returned default.", shaderSet.pipelineHandle.id);
        return pipelines[0];
    }
    return pipelines[shaderSet.pipelineHandle.id];
}

OpenGL::ShaderSet& OpenGL::get_shader_set(const Handle<ShaderSet> handle)
{
    if (handle.id >= buffers.size())
    {
        SPDLOG_ERROR("Shader set {} not found, returned default.", handle.id);
        return shaderSets[0];
    }
    return shaderSets[handle.id];
}

OpenGL::Image& OpenGL::get_image(const Handle<Image> handle)
{
    if (handle.id >= images.size())
    {
        SPDLOG_ERROR("Image {} not found, returned default.", handle.id);
        return images[0];
    }
    return images[handle.id];
}

OpenGL::Buffer& OpenGL::get_buffer(const Handle<Buffer> handle)
{
    if (handle.id >= buffers.size())
    {
        SPDLOG_ERROR("Buffer {} not found, returned default.", handle.id);
        return buffers[0];
    }
    return buffers[handle.id];
}

OpenGL::Buffer& OpenGL::get_array(const Handle<Buffer> handle)
{
    if (handle.id >= arrays.size())
    {
        SPDLOG_ERROR("Array {} not found, returned default.", handle.id);
        return arrays[0];
    }
    return arrays[handle.id];
}

Void OpenGL::gl_debug(UInt32 source, UInt32 type, UInt32 id, UInt32 severity, Int32 length, const Char* message, const Void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    {
        return;
    }

    SPDLOG_ERROR("Debug message ({}):", id);
    spdlog::set_pattern("%v");
    SPDLOG_ERROR("{}", message);
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
        {
            SPDLOG_INFO("Source: API");
            break;
        }
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        {
            SPDLOG_INFO("Source: Window System");
            break;
        }
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        {
            SPDLOG_INFO("Source: Shader Compiler");
            break;
        }
        case GL_DEBUG_SOURCE_THIRD_PARTY:
        {
            SPDLOG_INFO("Source: Third Party");
            break;
        }
        case GL_DEBUG_SOURCE_APPLICATION:
        {
            SPDLOG_INFO("Source: Application");
            break;
        }
        case GL_DEBUG_SOURCE_OTHER:
        {
            SPDLOG_INFO("Source: Other");
            break;
        }
        default:
        {
            SPDLOG_ERROR("NOT SUPPORTED");
            break;
        }
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
        {
            SPDLOG_INFO("Type: Error");
            break;
        }
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        {
            SPDLOG_INFO("Type: Deprecated Behaviour");
            break;
        }
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        {
            SPDLOG_INFO("Type: Undefined Behaviour");
            break;
        }
        case GL_DEBUG_TYPE_PORTABILITY:
        {
            SPDLOG_INFO("Type: Portability");
            break;
        }
        case GL_DEBUG_TYPE_PERFORMANCE:
        {
            SPDLOG_INFO("Type: Performance");
            break;
        }
        case GL_DEBUG_TYPE_MARKER:
        {
            SPDLOG_INFO("Type: Marker");
            break;
        }
        case GL_DEBUG_TYPE_PUSH_GROUP:
        {
            SPDLOG_INFO("Type: Push Group");
            break;
        }
        case GL_DEBUG_TYPE_POP_GROUP:
        {
            SPDLOG_INFO("Type: Pop Group");
            break;
        }
        case GL_DEBUG_TYPE_OTHER:
        {
            SPDLOG_INFO("Type: Other");
            break;
        }
        default:
        {
            SPDLOG_ERROR("NOT SUPPORTED");
            break;
        }
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        {
            SPDLOG_INFO("Severity: high");
            break;
        }
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
            SPDLOG_INFO("Severity: medium");
            break;
        }
        case GL_DEBUG_SEVERITY_LOW:
        {
            SPDLOG_INFO("Severity: low");
            break;
        }
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        {
            SPDLOG_INFO("Severity: notification");
            break;
        }
        default:
        {
            SPDLOG_ERROR("NOT SUPPORTED");
            break;
        }
    }

    spdlog::set_pattern("%+");
}

Void OpenGL::shutdown()
{
    if (!arrays.empty())
    {
        glDeleteVertexArrays(arrays.size(), arrays.data());
        arrays.clear();
    }
    if (!buffers.empty())
    {
        glDeleteBuffers(buffers.size(), buffers.data());
        buffers.clear();
    }
    if (!images.empty())
    {
        glDeleteTextures(images.size(), images.data());
        images.clear();
    }

    for (ShaderGL& shader : shaders)
    {
        shader.clear();
    }
    shaders.clear();
    shadersNameMap.clear();

    for (PipelineGL& pipeline : pipelines)
    {
        pipeline.clear();
    }
    pipelines.clear();
    shaderSets.clear();
}
