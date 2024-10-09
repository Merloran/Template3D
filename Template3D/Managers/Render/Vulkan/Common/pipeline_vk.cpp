#include "pipeline_vk.hpp"

#include "logical_device.hpp"
#include "descriptor_pool.hpp"
#include "render_pass.hpp"
#include "shader_vk.hpp"

#include <magic_enum.hpp>


Void PipelineVK::create_graphics_pipeline(const DescriptorPool& descriptorPool, const RenderPass& renderPass, const DynamicArray<ShaderVK>& ShaderVKs, const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    create_layout(descriptorPool.get_layouts(), descriptorPool.get_push_constants(), logicalDevice, allocator);

    DynamicArray<VkPipelineShaderStageCreateInfo> ShaderVKStageInfos;
    ShaderVKStageInfos.reserve(ShaderVKs.size());
    for (const ShaderVK& shader : ShaderVKs)
    {
        Bool isValid = create_shader_stage_info(shader, ShaderVKStageInfos.emplace_back());

        if (!isValid)
        {
            return;
        }
    }
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    DynamicArray<VkVertexInputBindingDescription> bindingDescriptions;
    get_mesh_binding_descriptions(bindingDescriptions);
    DynamicArray<VkVertexInputAttributeDescription> attributeDescriptions;
    get_mesh_attribute_descriptions(attributeDescriptions);
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = UInt32(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = UInt32(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = UInt32(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    if (renderPass.is_depth_test_enabled())
    {
	    depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	    depthStencil.depthTestEnable       = VK_TRUE;
	    depthStencil.depthWriteEnable      = VK_TRUE;
	    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
	    depthStencil.depthBoundsTestEnable = VK_FALSE;
	    depthStencil.minDepthBounds        = 0.0f; // Optional
	    depthStencil.maxDepthBounds        = 1.0f; // Optional
	    depthStencil.stencilTestEnable     = VK_FALSE;
	    depthStencil.front                 = {}; // Optional
	    depthStencil.back                  = {}; // Optional
    }

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp          = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor    = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = renderPass.get_samples();
    if (renderPass.is_multi_sampling_enabled())
    {
	    multisampling.sampleShadingEnable   = VK_TRUE; // enable sample shading in the pipeline
	    multisampling.minSampleShading      = 0.2f; // min fraction for sample shading; closer to one is smooth
	    multisampling.pSampleMask           = nullptr; // Optional
	    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	    multisampling.alphaToOneEnable      = VK_FALSE; // Optional
    } else {
        multisampling.sampleShadingEnable   = VK_FALSE; // enable sample shading in the pipeline
    }

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT
                                             | VK_COLOR_COMPONENT_G_BIT
                                             | VK_COLOR_COMPONENT_B_BIT
                                             | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = UInt32(ShaderVKStageInfos.size());
    pipelineInfo.pStages             = ShaderVKStageInfos.data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    if (renderPass.is_depth_test_enabled())
    {
        pipelineInfo.pDepthStencilState = &depthStencil;
    }
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = layout;
    pipelineInfo.renderPass          = renderPass.get_render_pass();
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex   = -1; // Optional

    VkResult result = vkCreateGraphicsPipelines(logicalDevice.get_device(),
                                                cache,
                                                1, 
                                                &pipelineInfo, 
                                                allocator, 
                                                &pipeline);

    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Creating graphics pipeline failed with: {}", magic_enum::enum_name(result));
        return;
    }
    type = EPipelineType::Graphics;
    bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
}

Void PipelineVK::create_compute_pipeline(const DescriptorPool& descriptorPool, const ShaderVK& shader, const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    create_layout(descriptorPool.get_layouts(), descriptorPool.get_push_constants(), logicalDevice, allocator);

    VkPipelineShaderStageCreateInfo ShaderVKStageInfo{};
    const Bool isValid = create_shader_stage_info(shader, ShaderVKStageInfo);

    if (!isValid)
    {
        return;
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout             = layout;
    pipelineInfo.stage              = ShaderVKStageInfo;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;

    const VkResult result = vkCreateComputePipelines(logicalDevice.get_device(),
                                                     cache,
                                                     1,
                                                     &pipelineInfo,
                                                     allocator,
                                                     &pipeline);

    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Creating compute pipeline failed with: {}", magic_enum::enum_name(result));
        return;
    }
    type = EPipelineType::Compute;
    bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
}

Void PipelineVK::recreate_pipeline(const DescriptorPool& descriptorPool, const RenderPass& renderPass, const DynamicArray<ShaderVK>& shaders, const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    clear(logicalDevice, allocator);
    if (type == EPipelineType::Graphics)
    {
        create_graphics_pipeline(descriptorPool, renderPass, shaders, logicalDevice, allocator);
    }
    else if (type == EPipelineType::Compute)
    {
        create_compute_pipeline(descriptorPool, shaders[0], logicalDevice, allocator);
    }
}

EPipelineType PipelineVK::get_type() const
{
    return type;
}

VkPipeline PipelineVK::get_pipeline() const
{
    return pipeline;
}

VkPipelineCache PipelineVK::get_cache() const
{
    return cache;
}

VkPipelineLayout PipelineVK::get_layout() const
{
    return layout;
}

VkPipelineBindPoint PipelineVK::get_bind_point() const
{
    return bindPoint;
}

Void PipelineVK::create_layout(const DynamicArray<VkDescriptorSetLayout>& descriptorSetLayout, const DynamicArray<VkPushConstantRange>& pushConstants, const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = UInt32(descriptorSetLayout.size());
    pipelineLayoutInfo.pSetLayouts            = descriptorSetLayout.data();
    pipelineLayoutInfo.pushConstantRangeCount = UInt32(pushConstants.size());
    pipelineLayoutInfo.pPushConstantRanges    = pushConstants.data();

    if (vkCreatePipelineLayout(logicalDevice.get_device(), &pipelineLayoutInfo, allocator, &layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline layout!");
    }
}

Bool PipelineVK::create_shader_stage_info(const ShaderVK& ShaderVK, VkPipelineShaderStageCreateInfo& info)
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    switch (ShaderVK.get_type())
    {
	    case EShaderType::Vertex: 
        {
            info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        }
	    case EShaderType::Geometry:
        {
            info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            break;
        }
	    case EShaderType::Fragment:
        {
            info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        }
	    case EShaderType::Compute:
        {
            info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        }
	    default:
	    {
            SPDLOG_ERROR("Not supported ShaderVK type {} in ShaderVK {}", 
						 magic_enum::enum_name(ShaderVK.get_type()), ShaderVK.get_name());
            return false;
	    }
    }
    info.module = ShaderVK.get_module();
    info.pName  = ShaderVK.get_function_name().c_str();
    return true;
}

Void PipelineVK::get_mesh_binding_descriptions(DynamicArray<VkVertexInputBindingDescription>& descriptions)
{
    descriptions.reserve(3);
    VkVertexInputBindingDescription& positionsBinding = descriptions.emplace_back();
    positionsBinding.binding = UInt32(descriptions.size()) - 1U;
    positionsBinding.stride = sizeof(FVector3);
    positionsBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputBindingDescription& normalsBinding = descriptions.emplace_back();
    normalsBinding.binding = UInt32(descriptions.size()) - 1U;
    normalsBinding.stride = sizeof(FVector3);
    normalsBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputBindingDescription& uvsBinding = descriptions.emplace_back();
    uvsBinding.binding = UInt32(descriptions.size()) - 1U;
    uvsBinding.stride = sizeof(FVector2);
    uvsBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

Void PipelineVK::get_mesh_attribute_descriptions(DynamicArray<VkVertexInputAttributeDescription>& descriptions)
{
    VkVertexInputAttributeDescription& positionsAttribute = descriptions.emplace_back();
    positionsAttribute.binding = UInt32(descriptions.size()) - 1U;
    positionsAttribute.location = UInt32(descriptions.size()) - 1U;
    positionsAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionsAttribute.offset = 0;

    VkVertexInputAttributeDescription& normalsAttribute = descriptions.emplace_back();
    normalsAttribute.binding = UInt32(descriptions.size()) - 1U;
    normalsAttribute.location = UInt32(descriptions.size()) - 1U;
    normalsAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalsAttribute.offset = 0;

    VkVertexInputAttributeDescription& uvsAttribute = descriptions.emplace_back();
    uvsAttribute.binding = UInt32(descriptions.size()) - 1U;
    uvsAttribute.location = UInt32(descriptions.size()) - 1U;
    uvsAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvsAttribute.offset = 0;
}

Void PipelineVK::clear(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    vkDestroyPipeline(logicalDevice.get_device(), pipeline, allocator);
    vkDestroyPipelineLayout(logicalDevice.get_device(), layout, allocator);
}