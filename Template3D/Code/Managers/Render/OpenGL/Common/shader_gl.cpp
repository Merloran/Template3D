#include "shader_gl.hpp"

#include <filesystem>
#include <fstream>
#include <magic_enum.hpp>
#include <glad/glad.h>

Bool ShaderGL::create(const String& path, const EShaderType shaderType)
{
    filePath = path;
    type = shaderType;
    compose_name();

    const Bool isLoaded = load();
    if (!isLoaded)
    {
        return false;
    }

    const Bool isCompiled = compile();
    if (!isCompiled)
    {
        return false;
    }

    return true;
}

Bool ShaderGL::create(const String& name, const String& shaderCode, const EShaderType shaderType)
{
    filePath = name;
    type = shaderType;
    code = shaderCode;
    compose_name();

    const Bool isCompiled = compile();
    if (!isCompiled)
    {
        return false;
    }

    return true;
}

Bool ShaderGL::recreate()
{
    clear();

    const Bool isLoaded = load();
    if (!isLoaded)
    {
        return false;
    }

    const Bool isCompiled = compile();
    if (!isCompiled)
    {
        return false;
    }

    return true;
}

const String& ShaderGL::get_name() const
{
    return name;
}
const String& ShaderGL::get_file_path() const
{
    return filePath;
}

UInt32 ShaderGL::get_module() const
{
    return module;
}

EShaderType ShaderGL::get_type() const
{
    return type;
}

Bool ShaderGL::has_compilation_errors() const
{
    Int32 success;
    String infoLog;
    infoLog.resize(1024);
    String shaderType(magic_enum::enum_name(type));

    glGetShaderiv(module, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(module, Int32(infoLog.size()), nullptr, infoLog.data());
        SPDLOG_ERROR("ERROR::SHADER_COMPILATION_ERROR::{}::{}\n{}", name, success, infoLog);
        return false;
    }

    return true;
}

Bool ShaderGL::load()
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        SPDLOG_ERROR("Failed to open shader {}", filePath);
        return false;
    }

    code.resize(file.tellg());
    file.seekg(0);
    file.read(code.data(), Int64(code.size()));
    file.close();

    return true;
}

Bool ShaderGL::compile()
{
    Int32 shaderType;
    switch (type)
    {
        case EShaderType::Vertex:
        {
            shaderType = GL_VERTEX_SHADER;
            break;
        }
        case EShaderType::Geometry:
        {
            shaderType = GL_GEOMETRY_SHADER;
            break;
        }
        case EShaderType::Fragment:
        {
            shaderType = GL_FRAGMENT_SHADER;
            break;
        }
        case EShaderType::Compute:
        {
            shaderType = GL_COMPUTE_SHADER;
            break;
        }
        case EShaderType::None:
        default:
        {
            SPDLOG_ERROR("Cannot compile shader type: {}", magic_enum::enum_name(type));
            return false;
        }
    }

    module = glCreateShader(shaderType);
    const char* codeSource = code.c_str();
    glShaderSource(module, 1, &codeSource, nullptr);
    glCompileShader(module);
    return has_compilation_errors();
}

Void ShaderGL::compose_name()
{
    String prefix;
    switch (type)
    {
    case EShaderType::Vertex:
    {
        prefix = "V";
        break;
    }
    case EShaderType::Geometry:
    {
        prefix = "G";
        break;
    }
    case EShaderType::Fragment:
    {
        prefix = "F";
        break;
    }
    case EShaderType::Compute:
    {
        prefix = "C";
        break;
    }
    case EShaderType::None:
    default:
    {
        prefix = "N";
        break;
    }
    }

    const std::filesystem::path path(filePath);
    name = prefix + path.stem().string();
}

Void ShaderGL::clear()
{
    if (module != 0)
    {
        glDeleteShader(module);
        module = 0;
    }
    code.clear();
    name.clear();
    filePath.clear();
    type = EShaderType::None;
}
