#include "pipeline_gl.hpp"

#include <magic_enum.hpp>
#include <glad/glad.h>

#include "shader_gl.hpp"

Bool PipelineGL::create_graphics_pipeline(const DynamicArray<ShaderGL>& shaders)
{
    type = EPipelineType::Graphics;
    pipeline = glCreateProgram();
    for (const ShaderGL& shader : shaders)
    {
        glAttachShader(pipeline, shader.get_module());
    }
    glLinkProgram(pipeline);

    return has_compilation_errors();
}

Bool PipelineGL::create_compute_pipeline(const ShaderGL& shader)
{
    type = EPipelineType::Compute;
    pipeline = glCreateProgram();
    glAttachShader(pipeline, shader.get_module());
    glLinkProgram(pipeline);

    return has_compilation_errors();
}

Bool PipelineGL::recreate_pipeline(const DynamicArray<ShaderGL>& shaders)
{
    if (shaders.empty())
    {
        return false;
    }

    clear();
    if (type == EPipelineType::Graphics)
    {
        return create_graphics_pipeline(shaders);
    }

    if (type == EPipelineType::Compute)
    {
        return create_compute_pipeline(shaders[0]);
    }

    return false;
}

Void PipelineGL::set_bool(const String& name, const Bool value)
{
    glUniform1i(glGetUniformLocation(pipeline, name.c_str()), Int32(value));
}

Void PipelineGL::set_int(const String& name, const Int32 value)
{
    glUniform1i(glGetUniformLocation(pipeline, name.c_str()), value);
}

Void PipelineGL::set_float(const String& name, const Float32 value)
{
    glUniform1f(glGetUniformLocation(pipeline, name.c_str()), value);
}

Void PipelineGL::set_vec2(const String& name, const Float32 x, const Float32 y)
{
    glUniform2f(glGetUniformLocation(pipeline, name.c_str()), x, y);
}

Void PipelineGL::set_vec2(const String& name, const FVector2& vector)
{
    glUniform2f(glGetUniformLocation(pipeline, name.c_str()), vector.x, vector.y);
}

Void PipelineGL::set_ivec2(const String& name, const Int32 x, const Int32 y)
{
    glUniform2i(glGetUniformLocation(pipeline, name.c_str()), x, y);
}

Void PipelineGL::set_ivec2(const String& name, const IVector2& vector)
{
    glUniform2i(glGetUniformLocation(pipeline, name.c_str()), vector.x, vector.y);
}

Void PipelineGL::set_vec3(const String& name, const Float32 x, const Float32 y, const Float32 z)
{
    glUniform3f(glGetUniformLocation(pipeline, name.c_str()), x, y, z);
}

Void PipelineGL::set_vec3(const String& name, const FVector3& vector)
{
    glUniform3f(glGetUniformLocation(pipeline, name.c_str()), vector.x, vector.y, vector.z);
}

Void PipelineGL::set_vec4(const String& name, Float32 x, Float32 y, Float32 z, Float32 w)
{
    glUniform4f(glGetUniformLocation(pipeline, name.c_str()), x, y, z, w);
}

Void PipelineGL::set_vec4(const String& name, const FVector4& vector)
{
    glUniform4f(glGetUniformLocation(pipeline, name.c_str()), vector.x, vector.y, vector.z, vector.w);
}

Void PipelineGL::set_mat4(const String& name, const FMatrix4& value)
{
    glUniformMatrix4fv(glGetUniformLocation(pipeline, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

Void PipelineGL::set_block(const String& name, const UInt32 number)
{
    glUniformBlockBinding(pipeline, glGetUniformBlockIndex(pipeline, name.c_str()), number);
}

Void PipelineGL::bind_uniform_buffer(const UInt32 uniformBufferObject, const UInt32 offset, const UInt32 size, const Float32* data)
{
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Void PipelineGL::bind() const
{
    if (activePipeline != pipeline)
    {
        glUseProgram(pipeline);
        activePipeline = pipeline;
    }
}

EPipelineType PipelineGL::get_type() const
{
    return type;
}

UInt32 PipelineGL::get_pipeline() const
{
    return pipeline;
}

Bool PipelineGL::has_compilation_errors() const
{
    Int32 success;
    String infoLog;
    infoLog.resize(1024);
    String shaderType(magic_enum::enum_name(type));

    glGetProgramiv(pipeline, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(pipeline, Int32(infoLog.size()), nullptr, infoLog.data());
        SPDLOG_ERROR("ERROR::PROGRAM_LINKING_ERROR::{}::{}\n{}", shaderType, success, infoLog);
        return false;
    }

    return true;
}

Void PipelineGL::clear()
{
    if (pipeline != 0)
    {
        glDeleteProgram(pipeline);
        pipeline = 0;
    }
}