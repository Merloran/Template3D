#pragma once
#include "../../Common/shader_type.hpp"

#include <vulkan/vulkan.hpp>

class LogicalDevice;
class ShaderVK
{
public:
    Void create(const String& shaderFilePath,
                const String& shaderFunctionName,
                const EShaderType shaderType,
                const LogicalDevice& logicalDevice,
                const VkAllocationCallbacks* allocator);
    Void create(const String& shaderName,
                const String& shaderCode,
                const String& shaderFunctionName,
                const EShaderType shaderType,
                const LogicalDevice& logicalDevice,
                const VkAllocationCallbacks* allocator);

    Bool recreate(const LogicalDevice& logicalDevice,
                  const VkAllocationCallbacks* allocator);

    [[nodiscard]]
    const String& get_name() const;
    [[nodiscard]]
    const String& get_function_name() const;
    [[nodiscard]]
    const String& get_file_path() const;
    [[nodiscard]]
    const VkShaderModule& get_module() const;
    [[nodiscard]]
    EShaderType get_type() const;

    Void clear(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator);

private:
    VkShaderModule module;
    String code;
    DynamicArray<UInt32> compiledCode;
    String filePath;
    String name, functionName;
    EShaderType type;

    Void compose_name();
    Bool compile();
    Bool load();
    Bool create_module(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator);
};