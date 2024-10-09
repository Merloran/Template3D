#pragma once
#include "../../Common/shader_type.hpp"

#include <vulkan/vulkan.hpp>

class LogicalDevice;

class ShaderVK
{
private:
    VkShaderModule module;
    String code;
    String filePath, destinationPath;
    String name, functionName;
    EShaderType type;

public:
    Void create(const String& filePath, 
				const String& destinationPath,
                const String& compilerPath,
                const String& functionName,
                const EShaderType shaderType, 
                const LogicalDevice& logicalDevice, 
                const VkAllocationCallbacks* allocator);

    Bool recreate(const String& compilerPath, 
                  const LogicalDevice& logicalDevice, 
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
    Void compose_name(const String& filePath, EShaderType type);
    Bool compile(const String& filePath, const String& destinationPath, const String& compilerPath);
    Bool load(const String& filePath, const String& destinationPath);
    Bool create_module(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator);
};