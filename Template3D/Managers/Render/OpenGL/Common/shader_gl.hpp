#pragma once
#include "../../Common/shader_type.hpp"

class ShaderGL
{
private:
    String name;
    String filePath;
    String code;
    EShaderType type;
    UInt32 module;

public:
    /** Create shader and give it a name as type prefix + fileName */
    Bool create(const String &path, const EShaderType shaderType);
    /** Create shader and give it a name as type prefix + shaderName, e.g. FDefault for type Fragment and shaderName Default */
    Bool create(const String &name, const String &shaderCode, const EShaderType shaderType);
    Bool recreate();

    [[nodiscard]]
    const String& get_name() const;
    [[nodiscard]]
    const String& get_file_path() const;
    [[nodiscard]]
    UInt32 get_module() const;
    [[nodiscard]]
    EShaderType get_type() const;

    Void clear();

private:
    Bool has_compilation_errors() const;
    Bool load();
    Bool compile();
    Void compose_name();
};

