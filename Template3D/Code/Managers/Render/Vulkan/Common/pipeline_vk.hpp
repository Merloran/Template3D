#pragma once
#include "Render/Common/pipeline_type.hpp"

#include <vulkan/vulkan.hpp>

class DescriptorPool;
class RenderPass;
class ShaderVK;
class LogicalDevice;

class PipelineVK
{
private:
    VkPipelineLayout layout;
    VkPipelineCache cache;
    VkPipeline pipeline;
    VkPipelineBindPoint bindPoint;
    EPipelineType type;
    Array<VkDynamicState, 2> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

public:
    Void create_graphics_pipeline(const DescriptorPool& descriptorPool,
                                  const RenderPass& renderPass,
                                  const DynamicArray<ShaderVK>& shaders,
                                  const LogicalDevice& logicalDevice,
                                  const VkAllocationCallbacks* allocator);

    Void create_compute_pipeline(const DescriptorPool& descriptorPool, 
                                 const ShaderVK& shader,
                                 const LogicalDevice& logicalDevice,
                                 const VkAllocationCallbacks* allocator);

    Void recreate_pipeline(const DescriptorPool& descriptorPool,
                           const RenderPass& renderPass,
                           const DynamicArray<ShaderVK>& shaders,
                           const LogicalDevice& logicalDevice,
                           const VkAllocationCallbacks* allocator);

    [[nodiscard]]
    EPipelineType get_type() const;
    [[nodiscard]]
    VkPipeline get_pipeline() const;
    [[nodiscard]]
    VkPipelineCache get_cache() const;
    [[nodiscard]]
    VkPipelineLayout get_layout() const;
    [[nodiscard]]
    VkPipelineBindPoint get_bind_point() const;

    Void clear(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator);

private:
    Void get_mesh_binding_descriptions(DynamicArray<VkVertexInputBindingDescription>& descriptions);
    Void get_mesh_attribute_descriptions(DynamicArray<VkVertexInputAttributeDescription>& descriptions);
    
    Void create_layout(const DynamicArray<VkDescriptorSetLayout>& descriptorSetLayouts,
                       const DynamicArray<VkPushConstantRange>& pushConstants,
                       const LogicalDevice& logicalDevice,
                       const VkAllocationCallbacks* allocator);
    Bool create_shader_stage_info(const ShaderVK& shader, VkPipelineShaderStageCreateInfo& info);
};
