#include "vulkan_api.hpp"

#include "Resource/Common/material.hpp"
#include "Resource/Common/texture.hpp"
#include "Resource/Common/mesh.hpp"
#include "Resource/Common/model.hpp"

#include <filesystem>
#include <GLFW/glfw3.h>
#include <magic_enum.hpp>
#include <glslang/Public/ShaderLang.h>

#include "simulation.hpp"


Void Vulkan::startup(Simulation<Vulkan>& simulation)
{
    isFrameEven = false;
    create_vulkan_instance();
    if constexpr (DebugMessenger::ENABLE_VALIDATION_LAYERS)
    {
        debugMessenger.create(instance, nullptr);
    }
    create_surface(simulation);
    physicalDevice.select_physical_device(instance, surface);
    logicalDevice.create(physicalDevice, debugMessenger, nullptr);

    create_dynamic_buffer<UniformBufferObject>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, "DefaultUniformBuffer");

    graphicsPool = create_command_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    create_command_buffers(get_command_pool(graphicsPool),
                           VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                           { "DefaultCommandBuffer" });

    inFlightFence = create_fence("DefaultFence", VK_FENCE_CREATE_SIGNALED_BIT);
    imageAvailable = create_semaphore("DefaultImageAvailable");
    renderFinished = create_semaphore("DefaultSemaphore");

    if (!glslang::InitializeProcess())
    {
        SPDLOG_ERROR("Failed to initialize glslang.");
        return;
    }
    // Create default pipeline
    {
        String vertCode = "#version 460                                                         \n"
                          "layout (location = 0) in vec3 position;                              \n"
                          "layout (location = 1) in vec3 normal;                                \n"
                          "layout (location = 2) in vec2 uv;                                    \n"
                          "                                                                     \n"
                          "                                                                     \n"
                          "layout(binding = 0) uniform UniformBufferObject                      \n"
                          "{                                                                    \n"
                          "    mat4 viewProjection;                                             \n"
                          "} ubo;                                                               \n"
                          "                                                                     \n"
                          "                                                                     \n"
                          "layout( push_constant ) uniform PushConstants                        \n"
                          "{                                                                    \n"
                          "    mat4 model;                                                      \n"
                          "} constants;                                                         \n"
                          "                                                                     \n"
                          "layout (location = 0) out vec3 worldPosition;                        \n"
                          "layout (location = 1) out vec3 worldNormal;                          \n"
                          "layout (location = 2) out vec2 uvFragment;                           \n"
                          "                                                                     \n"
                          "void main()                                                          \n"
                          "{                                                                    \n"
                          "    worldPosition = vec3(constants.model * vec4(position, 1.0f));    \n"
                          "    worldNormal = mat3(transpose(inverse(constants.model))) * normal;\n"
                          "    uvFragment = uv;                                                 \n"
                          "	                                                                    \n"
                          "	   gl_Position = ubo.viewProjection * vec4(worldPosition, 1.0f);    \n"
                          "}                                                                    \n";

        String fragCode = "#version 460                                                       \n"
                          "#extension GL_EXT_nonuniform_qualifier : require                   \n"
                          "                                                                   \n"
                          "layout (location = 0) in vec3 worldPosition;                       \n"
                          "layout (location = 1) in vec3 worldNormal;                         \n"
                          "layout (location = 2) in vec2 uvFragment;                          \n"
                          "                                                                   \n"
                          "layout(set = 1, binding = 0) uniform sampler2D Albedo;             \n"
                          "                                                                   \n"
                          "layout (location = 0) out vec4 color;                              \n"
                          "                                                                   \n"
                          "void main()                                                        \n"
                          "{                                                                  \n"
                          "    const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);                \n"
                          "    vec3 lightPosition = vec3(10.0f, 50.0f, 10.0f);                \n"
                          "    vec4 objectColor = texture(Albedo, uvFragment);                \n"
                          "    if (objectColor.w < 0.1f)                                      \n"
                          "    {                                                              \n"
                          "        discard;                                                   \n"
                          "    }                                                              \n"
                          "                                                                   \n"
                          "    // ambient                                                     \n"
                          "    float ambientStrength = 0.5f;                                  \n"
                          "    vec3 ambient = ambientStrength * lightColor;                   \n"
                          "    // diffuse                                                     \n"
                          "    vec3 normal = normalize(worldNormal);                          \n"
                          "    vec3 lightDirection = normalize(lightPosition - worldPosition);\n"
                          "    float diff = max(dot(normal, lightDirection), 0.0f);           \n"
                          "    vec3 diffuse = diff * lightColor;                              \n"
                          "                                                                   \n"
                          "    color = vec4((ambient + diffuse) * objectColor.xyz, 1.0f);     \n"
                          "}                                                                  \n";


        ShaderSet defaultSet;
        //Shaders should be created after logical device
        defaultSet.shaderHandles.push_back(create_shader("Default", vertCode, EShaderType::Vertex));
        defaultSet.shaderHandles.push_back(create_shader("Default", fragCode, EShaderType::Fragment));

        swapchain.create(logicalDevice,
                         physicalDevice,
                         surface, 
                         simulation.displayManager.get_framebuffer_size(), 
                         nullptr);

        defaultSet.renderPassHandle = create_render_pass(physicalDevice.get_max_samples());
        defaultSet.descriptorPoolHandle = create_descriptor_pool();
        create_default_descriptors();

        defaultSet.pipelineHandle = create_pipeline(defaultSet);
        simulation.resourceManager.get_default_material().shaderSetHandle = create_shader_set(defaultSet);
        create_model_render_data(simulation, simulation.resourceManager.get_default_model());
        setup_default_descriptors(simulation);
    }
}

Void Vulkan::draw_model(Simulation<Vulkan>& simulation, const Model<Vulkan>& model)
{
    const VkFence renderFence = get_fence(inFlightFence);
    logicalDevice.wait_for_fence(renderFence, true);
    logicalDevice.reset_fence(renderFence);

    CommandBuffer commandBuffer = get_command_buffer("DefaultCommandBuffer");
    const UVector2& extent = swapchain.get_extent();
    VkSemaphore imageSemaphore = get_semaphore(imageAvailable);
    VkResult result = logicalDevice.acquire_next_image(swapchain, imageSemaphore);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swapchain(simulation);
        return;
    }

    {
        UniformBufferObject ubo{};
        const FMatrix4 projectionMatrix = glm::perspective(glm::radians(70.0f),
                                                           simulation.displayManager.get_aspect_ratio(),
                                                           0.001f,
                                                           5000.0f);
        const FMatrix4 viewMatrix = glm::lookAt(FVector3{ 0.0f, 0.0f, -10.0f },
                                                FVector3{ 0.0f, 0.0f, 0.0f },
                                                FVector3{ 0.0f, 1.0f, 0.0f });
        ubo.viewProjection = projectionMatrix * viewMatrix;

        get_buffer("DefaultUniformBuffer").update_dynamic_buffer(ubo);
    }

    ResourceManager<Vulkan>& resourceManager = simulation.resourceManager;
    for (UInt64 i = 0; i < model.meshes.size(); ++i)
    {
        const Mesh<Vulkan>& mesh = resourceManager.get_mesh(model.meshes[i]);
        const Material<Vulkan>& material = resourceManager.get_material(model.materials[i]);
        ShaderSet& shaderSet = get_shader_set(material.shaderSetHandle);
        Pipeline& pipeline = get_pipeline(shaderSet.pipelineHandle);
        RenderPass& renderPass = get_render_pass(shaderSet.renderPassHandle);
        DescriptorPool& descriptorPool = get_descriptor_pool(shaderSet.descriptorPoolHandle);

        commandBuffer.reset(0);
        commandBuffer.begin();
        commandBuffer.begin_render_pass(renderPass,
                                        swapchain,
                                        swapchain.get_image_index(),
                                        VK_SUBPASS_CONTENTS_INLINE);
        commandBuffer.bind_pipeline(pipeline);

        commandBuffer.set_viewport(0, { 0.0f, 0.0f }, extent, { 0.0f, 1.0f });
        commandBuffer.set_scissor(0, { 0, 0 }, extent);

        DescriptorResourceInfo textureResource;
        VkDescriptorImageInfo& textureInfo = textureResource.imageInfos.emplace_back();
        Texture<Vulkan> albedoTexture = resourceManager.get_texture(material[ETextureType::Albedo]);
        const Image& albedo = get_image(albedoTexture.imageHandle);
        textureInfo.imageLayout = albedo.get_current_layout();
        textureInfo.imageView = albedo.get_view();
        textureInfo.sampler = albedo.get_sampler();

        descriptorPool.update_set(logicalDevice, 
                                  textureResource,
                                  "Texture", 
                                  0, 
                                  0);

        const DescriptorSetData& uniformSet = descriptorPool.get_set_data("Uniforms");
        commandBuffer.bind_descriptor_set(pipeline, uniformSet.set, uniformSet.setNumber);

        const DescriptorSetData& textureSet = descriptorPool.get_set_data("Texture");
        commandBuffer.bind_descriptor_set(pipeline, textureSet.set, textureSet.setNumber);
    

        const VkBuffer vertexesBuffer = get_buffer(mesh.vertexesHandle).get_buffer();
        const VkBuffer indexesBuffer = get_buffer(mesh.indexesHandle).get_buffer();
        
        commandBuffer.bind_vertex_buffers<1>(0, { vertexesBuffer }, { 0 });
        commandBuffer.bind_index_buffer(indexesBuffer, 0, VK_INDEX_TYPE_UINT32);

        VertexConstants vertexConstants{};
        static Float32 rot = 0.0f;
        vertexConstants.model = glm::rotate(FMatrix4(1.0f), 
                                            glm::radians(rot), 
                                            {0.0f, 1.0f, 0.0f});
        rot += 0.01f;

        commandBuffer.set_constants(pipeline,
                                    VK_SHADER_STAGE_VERTEX_BIT,
                                    0,
                                    sizeof(vertexConstants),
                                    &vertexConstants);


        commandBuffer.draw_indexed(UInt32(mesh.indexes.size()),
                                   1,
                                   0,
                                   0,
                                   0);
    

        commandBuffer.end_render_pass();
        commandBuffer.end();

        const VkSemaphore renderSemaphore = get_semaphore(renderFinished);
        logicalDevice.submit_graphics_queue(imageSemaphore,
                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                            commandBuffer.get_buffer(),
                                            renderSemaphore,
                                            renderFence);

        const VkResult result = logicalDevice.submit_present_queue(renderSemaphore, swapchain);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            recreate_swapchain(simulation);
        }
    }

    isFrameEven = !isFrameEven;
}

Handle<Vulkan::Shader> Vulkan::create_shader(const String& filePath, const EShaderType shaderType, const String& functionName)
{
    const UInt64 shaderId = shaders.size();
    Shader& shader = shaders.emplace_back();

    shader.create(filePath, functionName, shaderType, logicalDevice, nullptr);

    const Handle<Shader> handle{ shaderId };
    auto iterator = shadersNameMap.find(shader.get_name());
    if (iterator != shadersNameMap.end())
    {
        SPDLOG_WARN("Shader {} already exists.", shader.get_name());
        shader.clear(logicalDevice, nullptr);
        return iterator->second;
    }

    shadersNameMap[shader.get_name()] = handle;

    return handle;
}

Handle<Vulkan::Shader> Vulkan::create_shader(const String& shaderName, const String& shaderCode, EShaderType shaderType, const String& functionName)
{
    const UInt64 shaderId = shaders.size();
    Shader& shader = shaders.emplace_back();

    shader.create(shaderName, shaderCode, functionName, shaderType, logicalDevice, nullptr);

    const Handle<Shader> handle{ shaderId };
    auto iterator = shadersNameMap.find(shader.get_name());
    if (iterator != shadersNameMap.end())
    {
        SPDLOG_WARN("Shader {} already exists.", shader.get_name());
        shader.clear(logicalDevice, nullptr);
        return iterator->second;
    }

    shadersNameMap[shader.get_name()] = handle;

    return handle;
}

Handle<Vulkan::Pipeline> Vulkan::create_pipeline(const ShaderSet& shaderSet)
{
    Handle<Pipeline> handle = { pipelines.size() };
    Pipeline& pipeline = pipelines.emplace_back();

    const DescriptorPool& descriptorPool = get_descriptor_pool(shaderSet.descriptorPoolHandle);
    const RenderPass& renderPass = get_render_pass(shaderSet.renderPassHandle);
    DynamicArray<Shader> pipelineShaders;
    pipelineShaders.reserve(shaderSet.shaderHandles.size());
    for (const Handle<ShaderVK> shaderHandle : shaderSet.shaderHandles)
    {
        pipelineShaders.push_back(get_shader(shaderHandle));
    }

    pipeline.create_graphics_pipeline(descriptorPool,
                                      renderPass,
                                      pipelineShaders,
                                      logicalDevice,
                                      nullptr);

    return handle;
}

Handle<Vulkan::ShaderSet> Vulkan::create_shader_set(const ShaderSet& shaderSet)
{
    const UInt64 shaderSetId = shaderSets.size();
    shaderSets.push_back(shaderSet);
    const Handle<ShaderSet> handle{ shaderSetId };
    return handle;
}

Void Vulkan::create_model_render_data(Simulation<Vulkan>& simulation, Model<Vulkan>& model)
{
    for (UInt64 i = 0; i < model.meshes.size(); ++i)
    {
        create_mesh_buffers(simulation.resourceManager.get_mesh(model.meshes[i]));
        create_material_images(simulation, simulation.resourceManager.get_material(model.materials[i]));
    }
}

Void Vulkan::create_mesh_buffers(Mesh<Vulkan>& mesh)
{
    mesh.vertexesHandle = create_static_buffer(mesh.vertexes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    mesh.indexesHandle  = create_static_buffer(mesh.indexes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

Void Vulkan::create_material_images(Simulation<Vulkan>& simulation, Material<Vulkan>& material)
{
    for (Handle<Texture<Vulkan>>& textureHandle : material.textures)
    {
        if (textureHandle != Handle<Texture<Vulkan>>::NONE)
        {
            Texture<Vulkan>& texture = simulation.resourceManager.get_texture(textureHandle);
            UInt32 mipLevels = UInt32(std::floor(std::log2(std::max(texture.size.x, texture.size.y)))) + 1;
            create_texture_image(texture, mipLevels);
        }
    }
}

Void Vulkan::create_texture_image(Texture<Vulkan>& texture, UInt32 mipLevels)
{
    Buffer stagingBuffer{};
    UInt64 textureSize = UInt64(texture.size.x * texture.size.y * texture.channels);
    if (texture.type == ETextureType::HDR)
    {
        textureSize *= sizeof(Float32);
    }
    stagingBuffer.create(physicalDevice,
                         logicalDevice,
                         textureSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         nullptr);

    Void* data;
    vkMapMemory(logicalDevice.get_device(), stagingBuffer.get_memory(), 0, textureSize, 0, &data);
    memcpy(data, texture.data, textureSize);
    vkUnmapMemory(logicalDevice.get_device(), stagingBuffer.get_memory());


    if (texture.channels != 4)
    {
        SPDLOG_ERROR("Not supported channels count: {} in texture: {}", texture.channels, texture.name);
        stagingBuffer.clear(logicalDevice, nullptr);
        return;
    }
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    if (texture.type == ETextureType::HDR)
    {
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }


    texture.imageHandle.id = images.size();
    Image& textureImage = images.emplace_back();

    textureImage.create(physicalDevice,
                        logicalDevice,
                        texture.size,
                        mipLevels,
                        VK_SAMPLE_COUNT_1_BIT,
                        format,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        nullptr);
    textureImage.create_sampler(physicalDevice, logicalDevice, nullptr);

    transition_image_layout(textureImage,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(stagingBuffer, textureImage);
    if (texture.type != ETextureType::HDR)
    {
        generate_mipmaps(textureImage); // implicit transition to read optimal
    }
    else {
        transition_image_layout(textureImage,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    stagingBuffer.clear(logicalDevice, nullptr);
}

Void Vulkan::load_pixels_from_image(Texture<Vulkan>& texture)
{
    Buffer buffer{};
    Image& image = get_image(texture.imageHandle);
    UInt64 pixelSize;
    const UVector2 imageSize = image.get_size();
    UInt8 type; // 0 - int8, 1 - float32
    switch (image.get_format())
    {
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_R8G8B8_UINT:
    {
        pixelSize = 3 * sizeof(UInt8);
        break;
    }
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_R8G8B8_SRGB:
    {
        pixelSize = 3 * sizeof(Int8);
        break;
    }
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_R8G8B8A8_UINT:
    {
        pixelSize = 4 * sizeof(UInt8);
        break;
    }
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_R8G8B8A8_SRGB:
    {
        pixelSize = 4 * sizeof(Int8);
        break;
    }
    case VK_FORMAT_R32G32B32_UINT:
    {
        pixelSize = 3 * sizeof(UInt32);
        break;
    }
    case VK_FORMAT_R32G32B32_SINT:
    {
        pixelSize = 3 * sizeof(Int32);
        break;
    }
    case VK_FORMAT_R32G32B32_SFLOAT:
    {
        pixelSize = 3 * sizeof(Float32);
        type = 1;
        break;
    }
    case VK_FORMAT_R32G32B32A32_UINT:
    {
        pixelSize = 4 * sizeof(UInt32);
        break;
    }
    case VK_FORMAT_R32G32B32A32_SINT:
    {
        pixelSize = 4 * sizeof(Int32);
        break;
    }
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    {
        pixelSize = 4 * sizeof(Float32);
        type = 1;
        break;
    }
    case VK_FORMAT_R64G64B64_UINT:
    {
        pixelSize = 3 * sizeof(UInt64);
        break;
    }
    case VK_FORMAT_R64G64B64_SINT:
    {
        pixelSize = 3 * sizeof(Int64);
        break;
    }
    case VK_FORMAT_R64G64B64_SFLOAT:
    {
        pixelSize = 3 * sizeof(Float64);
        break;
    }
    case VK_FORMAT_R64G64B64A64_UINT:
    {
        pixelSize = 4 * sizeof(UInt64);
        break;
    }
    case VK_FORMAT_R64G64B64A64_SINT:
    {
        pixelSize = 4 * sizeof(Int64);
        break;
    }
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    {
        pixelSize = 4 * sizeof(Float64);
        break;
    }
    default:
    {
        SPDLOG_ERROR("Not supported image format: {}, copy to buffer failed.", magic_enum::enum_name(image.get_format()));
        return;
    }
    }

    const VkDeviceSize size = pixelSize * imageSize.x * imageSize.y;

    buffer.create(physicalDevice,
                  logicalDevice,
                  size,
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  nullptr);

    logicalDevice.wait_idle();
    copy_image_to_buffer(buffer, image);
    //TODO: Sorry code for my sin ;-; don't judge my laziness ;-;
    if (texture.data == nullptr)
    {
        texture.data = reinterpret_cast<UInt8*>(malloc(texture.channels * texture.size.x * texture.size.y));
        if (texture.data == nullptr)
        {
            SPDLOG_ERROR("Failed to allocate texture memory");
            free(texture.data);
            return;
        }
    } else {
        texture.data = reinterpret_cast<UInt8*>(realloc(texture.data, texture.channels * texture.size.x * texture.size.y));
        if (texture.data == nullptr)
        {
            SPDLOG_ERROR("Failed to allocate texture memory");
            free(texture.data);
            return;
        }
    }

    switch (type)
    {
        case 1:
        {
            DynamicArray<Float32> tempData;
            tempData.resize(size / sizeof(Float32));

            vkMapMemory(logicalDevice.get_device(), buffer.get_memory(), 0, size, 0, buffer.get_mapped_memory());
            memcpy(tempData.data(), *buffer.get_mapped_memory(), size);
            vkUnmapMemory(logicalDevice.get_device(), buffer.get_memory());

            for (UInt64 i = 0; i < tempData.size(); ++i)
            {
                texture.data[i] = UInt8(glm::clamp(glm::pow(tempData[i], 1.0f / 2.2f), 0.0f, 1.0f) * 255.0f);
            }

            tempData.clear();
            break;
        }
        default:
        {
            SPDLOG_ERROR("Not supported image type, failed to copy pixels");
            return;
        }
    }

}

Handle<Vulkan::Image> Vulkan::create_image(const UVector2& size, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling, UInt32 mipLevels)
{
    const Handle<Image> handle = { images.size() };
    Image& image = images.emplace_back();

    image.create(physicalDevice,
                 logicalDevice,
                 size,
                 mipLevels,
                 VK_SAMPLE_COUNT_1_BIT,
                 format,
                 tiling,
                 usage,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 VK_IMAGE_ASPECT_COLOR_BIT,
                 nullptr);

    image.create_sampler(physicalDevice, logicalDevice, nullptr);

    return handle;
}

Handle<RenderPass> Vulkan::create_render_pass(VkSampleCountFlagBits samples, Bool depthTest, VkAttachmentLoadOp loadOperation)
{
    Handle<RenderPass> handle = { renderPasses.size() };
    RenderPass& pass = renderPasses.emplace_back();
    pass.create(physicalDevice, logicalDevice, swapchain, nullptr, samples, depthTest, loadOperation);
    return handle;
}

Handle<DescriptorPool> Vulkan::create_descriptor_pool()
{
    Handle<DescriptorPool> handle = { descriptorPools.size() };
    descriptorPools.emplace_back();
    return handle;
}

Handle<VkCommandPool> Vulkan::create_command_pool(VkCommandPoolCreateFlagBits flags)
{
    Handle<VkCommandPool> handle = { commandPools.size() };
    VkCommandPool pool;
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flags;
    poolInfo.queueFamilyIndex = physicalDevice.get_graphics_family_index();

    const VkResult result = vkCreateCommandPool(logicalDevice.get_device(), &poolInfo, nullptr, &pool);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Creating command pool failed with: {}", magic_enum::enum_name(result));
        return Handle<VkCommandPool>::NONE;
    }

    commandPools.push_back(pool);
    return handle;
}

Void Vulkan::create_command_buffers(Handle<VkCommandPool> handle, VkCommandBufferLevel level, const DynamicArray<String>& names)
{
    create_command_buffers(get_command_pool(handle), level, names);
}

Void Vulkan::create_command_buffers(VkCommandPool pool, VkCommandBufferLevel level, const DynamicArray<String>& names)
{
    for (const String& name : names)
    {
        const auto& iterator = commandBuffersNameMap.find(name);
        if (iterator != commandBuffersNameMap.end())
        {
            SPDLOG_ERROR("Failed to create command buffers, name: {} already exist.", name);
            return;
        }
    }
    commandBuffers.reserve(names.size());
    DynamicArray<VkCommandBuffer> buffers;
    buffers.resize(names.size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = UInt32(buffers.size());

    const VkResult result = vkAllocateCommandBuffers(logicalDevice.get_device(), 
                                                     &allocInfo, 
                                                     buffers.data());
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Creating command buffers failed with: {}", magic_enum::enum_name(result));
        return;
    }

    for (UInt64 i = 0; i < names.size(); ++i)
    {
        const Handle<CommandBuffer> handle = { commandBuffers.size() };
        commandBuffersNameMap[names[i]] = handle;
        CommandBuffer& buffer = commandBuffers.emplace_back();
        buffer.set_buffer(buffers[i]);
        buffer.set_name(names[i]);
    }
}

Handle<VkSemaphore> Vulkan::create_semaphore(const String& name)
{
    if (semaphoresNameMap.contains(name))
    {
        SPDLOG_ERROR("Semaphore: {}, already exists, returned None", name);
        return Handle<VkSemaphore>::NONE;
    }
    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    const Handle<VkSemaphore> handle = { semaphores.size() };
    VkSemaphore semaphore;
    const VkResult result = vkCreateSemaphore(logicalDevice.get_device(), &info, nullptr, &semaphore);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Semaphore: {}, creation failed with: {}, returned None", name, magic_enum::enum_name(result));
        return Handle<VkSemaphore>::NONE;
    }

    semaphores.push_back(semaphore);
    semaphoresNameMap[name] = handle;
    return handle;
}

Handle<VkFence> Vulkan::create_fence(const String& name, VkFenceCreateFlags flags)
{
    if (fencesNameMap.contains(name))
    {
        SPDLOG_ERROR("Fence: {}, already exists, returned None", name);
        return Handle<VkFence>::NONE;
    }
    VkFenceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = flags;
    const Handle<VkFence> handle = { fences.size() };
    VkFence fence;
    const VkResult result = vkCreateFence(logicalDevice.get_device(), &info, nullptr, &fence);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Fence: {}, creation failed with: {}", name, magic_enum::enum_name(result));
        return Handle<VkFence>::NONE;
    }

    fences.push_back(fence);
    fencesNameMap[name] = handle;
    return handle;
}



VkSurfaceKHR Vulkan::get_surface() const
{
    return surface;
}

const PhysicalDevice& Vulkan::get_physical_device() const
{
    return physicalDevice;
}

const LogicalDevice& Vulkan::get_logical_device() const
{
    return logicalDevice;
}

Swapchain& Vulkan::get_swapchain()
{
    return swapchain;
}

const Handle<Vulkan::Shader>& Vulkan::get_shader_handle(const String& name) const
{
    const auto& iterator = shadersNameMap.find(name);
    if (iterator == shadersNameMap.end())
    {
        SPDLOG_WARN("Shader handle {} not found, returned None.", name);
        return Handle<Shader>::NONE;
    }

    return iterator->second;
}

Vulkan::Shader& Vulkan::get_shader(const String& name)
{
    const auto& iterator = shadersNameMap.find(name);
    if (iterator == shadersNameMap.end())
    {
        SPDLOG_WARN("Shader {} not found, returned default.", name);
        return shaders[0];
    }

    return shaders[iterator->second.id];
}

Vulkan::Shader& Vulkan::get_shader(const Handle<Shader> handle)
{
    if (handle.id >= shaders.size())
    {
        SPDLOG_WARN("Shader {} not found, returned default.", handle.id);
        return shaders[0];
    }
    return shaders[handle.id];
}

Vulkan::Pipeline& Vulkan::get_pipeline(const Handle<Pipeline> handle)
{
    if (handle.id >= buffers.size())
    {
        SPDLOG_ERROR("Pipeline {} not found, returned default.", handle.id);
        return pipelines[0];
    }
    return pipelines[handle.id];
}

Vulkan::ShaderSet& Vulkan::get_shader_set(const Handle<ShaderSet> handle)
{
    if (handle.id >= buffers.size())
    {
        SPDLOG_ERROR("Shader set {} not found, returned default.", handle.id);
        return shaderSets[0];
    }
    return shaderSets[handle.id];
}

const Handle<CommandBuffer>& Vulkan::get_command_buffer_handle(const String& name) const
{
    const auto& iterator = commandBuffersNameMap.find(name);
    if (iterator == commandBuffersNameMap.end())
    {
        SPDLOG_ERROR("Command buffer handle {} not found, returned None.", name);
        return Handle<CommandBuffer>::NONE;
    }

    return iterator->second;
}

CommandBuffer& Vulkan::get_command_buffer(const String& name)
{
    const auto& iterator = commandBuffersNameMap.find(name);
    if (iterator == commandBuffersNameMap.end())
    {
        SPDLOG_ERROR("Command buffer {} not found, returned default.", name);
        return commandBuffers[0];
    }

    return commandBuffers[iterator->second.id];
}

CommandBuffer& Vulkan::get_command_buffer(const Handle<CommandBuffer> handle)
{
    if (handle.id >= commandBuffers.size())
    {
        SPDLOG_ERROR("Command buffer {} not found, returned default.", handle.id);
        return commandBuffers[0];
    }
    return commandBuffers[handle.id];
}

const Handle<VkSemaphore>& Vulkan::get_semaphore_handle(const String& name) const
{
    const auto& iterator = semaphoresNameMap.find(name);
    if (iterator == semaphoresNameMap.end())
    {
        SPDLOG_ERROR("Semaphore handle {} not found, returned None.", name);
        return Handle<VkSemaphore>::NONE;
    }

    return iterator->second;
}

VkSemaphore& Vulkan::get_semaphore(const String& name)
{
    const auto& iterator = semaphoresNameMap.find(name);
    if (iterator == semaphoresNameMap.end())
    {
        SPDLOG_ERROR("Semaphore {} not found, returned default.", name);
        return semaphores[0];
    }

    return semaphores[iterator->second.id];
}

VkSemaphore& Vulkan::get_semaphore(const Handle<VkSemaphore> handle)
{
    if (handle.id >= semaphores.size())
    {
        SPDLOG_ERROR("Semaphore {} not found, returned default.", handle.id);
        return semaphores[0];
    }
    return semaphores[handle.id];
}

const Handle<VkFence>& Vulkan::get_fence_handle(const String& name) const
{
    const auto& iterator = fencesNameMap.find(name);
    if (iterator == fencesNameMap.end())
    {
        SPDLOG_ERROR("Fence handle {} not found, returned None.", name);
        return Handle<VkFence>::NONE;
    }

    return iterator->second;
}

VkFence& Vulkan::get_fence(const String& name)
{
    const auto& iterator = fencesNameMap.find(name);
    if (iterator == fencesNameMap.end())
    {
        SPDLOG_ERROR("Fence {} not found, returned default.", name);
        return fences[0];
    }

    return fences[iterator->second.id];
}

VkFence& Vulkan::get_fence(const Handle<VkFence> handle)
{
    if (handle.id >= fences.size())
    {
        SPDLOG_ERROR("Fence {} not found, returned default.", handle.id);
        return fences[0];
    }
    return fences[handle.id];
}

Vulkan::Image& Vulkan::get_image(const Handle<Image> handle)
{
    if (handle.id >= images.size())
    {
        SPDLOG_ERROR("Image {} not found, returned default.", handle.id);
        return images[0];
    }
    return images[handle.id];
}

const Handle<Vulkan::Buffer>& Vulkan::get_buffer_handle(const String& name) const
{
    const auto& iterator = buffersNameMap.find(name);
    if (iterator == buffersNameMap.end())
    {
        SPDLOG_ERROR("Buffer handle {} not found, returned None.", name);
        return Handle<Buffer>::NONE;
    }

    return iterator->second;
}

Vulkan::Buffer& Vulkan::get_buffer(const Handle<Buffer> handle)
{
    if (handle.id >= buffers.size())
    {
        SPDLOG_ERROR("Buffer {} not found, returned default.", handle.id);
        return buffers[0];
    }
    return buffers[handle.id];
}

Vulkan::Buffer& Vulkan::get_buffer(const String& name)
{
    const auto& iterator = buffersNameMap.find(name);
    if (iterator == buffersNameMap.end())
    {
        SPDLOG_ERROR("Buffer {} not found, returned default.", name);
        return buffers[0];
    }

    return buffers[iterator->second.id];
}

VkCommandPool& Vulkan::get_command_pool(const Handle<VkCommandPool> handle)
{
    if (handle.id >= commandPools.size())
    {
        SPDLOG_ERROR("Command pool {} not found, returned default.", handle.id);
        return commandPools[0];
    }
    return commandPools[handle.id];
}

RenderPass& Vulkan::get_render_pass(const Handle<RenderPass> handle)
{
    if (handle.id >= renderPasses.size())
    {
        SPDLOG_ERROR("Render pass {} not found, returned default.", handle.id);
        return renderPasses[0];
    }
    return renderPasses[handle.id];
}

DescriptorPool& Vulkan::get_descriptor_pool(const Handle<DescriptorPool> handle)
{
    if (handle.id >= descriptorPools.size())
    {
        SPDLOG_ERROR("Descriptor Pool {} not found, returned default.", handle.id);
        return descriptorPools[0];
    }
    return descriptorPools[handle.id];
}

DescriptorPool& Vulkan::get_default_descriptor_pool()
{
    return descriptorPools[0];
}

Void Vulkan::create_vulkan_instance()
{
    if constexpr (DebugMessenger::ENABLE_VALIDATION_LAYERS)
    {
        if (!debugMessenger.check_validation_layer_support())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Ray Tracer";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0U, 1U, 0U, 0U);
    appInfo.pEngineName        = "RayEngine";
    appInfo.engineVersion      = VK_MAKE_API_VERSION(0U, 1U, 0U, 0U);
    appInfo.apiVersion         = VK_MAKE_API_VERSION(0U, 1U, 3U, 0U);

    VkInstanceCreateInfo createInfo{};
    const DynamicArray<const Char*> extensions = get_required_extensions();

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    if constexpr (DebugMessenger::ENABLE_VALIDATION_LAYERS)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugMessenger.fill_debug_messenger_create_info(debugCreateInfo);
        const DynamicArray<const Char*>& validationLayers = debugMessenger.get_validation_layers();
        createInfo.enabledLayerCount = UInt32(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        createInfo.pNext = &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0U;
        createInfo.pNext = nullptr;
    }
    createInfo.enabledExtensionCount = UInt32(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    //TODO: in the far future think about using custom allocator
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

Void Vulkan::create_surface(Simulation<Vulkan>& simulation)
{
    VkResult result = glfwCreateWindowSurface(instance, 
                                              simulation.displayManager.get_current_window(), 
                                              nullptr, 
                                              &surface);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("failed to create window surface!");
        simulation.shutdown();
    }
}

DynamicArray<const Char*> Vulkan::get_required_extensions()
{
    UInt32 glfwExtensionCount = 0U;
    const Char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    DynamicArray<const Char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if constexpr (DebugMessenger::ENABLE_VALIDATION_LAYERS)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


Void Vulkan::create_default_descriptors()
{
    DescriptorPool& descriptorPool = get_default_descriptor_pool();
    descriptorPool.add_binding("TextureData",
                               1,
                               0,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                               1,
                               VK_SHADER_STAGE_FRAGMENT_BIT,
                               VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                               VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                               VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    descriptorPool.add_binding("ViewProjection",
                               0,
                               0,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                               1,
                               VK_SHADER_STAGE_VERTEX_BIT,
                               VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                               VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                               VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);


    descriptorPool.create_layouts(logicalDevice, nullptr);

    DynamicArray<VkPushConstantRange> pushConstants;
    VkPushConstantRange& vertexConstant = pushConstants.emplace_back();
    vertexConstant.size = sizeof(VertexConstants);
    vertexConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    descriptorPool.set_push_constants(pushConstants);
}

Void Vulkan::setup_default_descriptors(Simulation<Vulkan>& simulation)
{
    DescriptorPool& descriptorPool = get_default_descriptor_pool();

    DynamicArray<DescriptorResourceInfo> uniformResources;
    VkDescriptorBufferInfo& uniformBufferInfo = uniformResources.emplace_back().bufferInfos.emplace_back();
    uniformBufferInfo.buffer = get_buffer("DefaultUniformBuffer").get_buffer();
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(UniformBufferObject);

    descriptorPool.add_set(descriptorPool.get_layout_data_handle("ViewProjection"),
                           uniformResources,
                           "Uniforms");

    DynamicArray<DescriptorResourceInfo> resources;
    DynamicArray<VkDescriptorImageInfo>& imageInfos = resources.emplace_back().imageInfos;

    Handle<Texture<Vulkan>> textureHandle = simulation.resourceManager.get_default_material()[ETextureType::Albedo];
    const Texture<Vulkan>& texture = simulation.resourceManager.get_texture(textureHandle);

    Image& image = get_image(texture.imageHandle);
    VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
    imageInfo.imageLayout = image.get_current_layout();
    imageInfo.imageView   = image.get_view();
    imageInfo.sampler     = image.get_sampler();

    descriptorPool.add_set(descriptorPool.get_layout_data_handle("TextureData"),
                           resources,
                           "Texture");

    descriptorPool.create_sets(logicalDevice, nullptr);
}

Void Vulkan::recreate_swapchain(Simulation<Vulkan>& simulation)
{
    IVector2 windowSize = simulation.displayManager.get_framebuffer_size();
    while (windowSize.x < 1 || windowSize.y < 1)
    {
        windowSize = simulation.displayManager.get_framebuffer_size();
        glfwWaitEvents();
    }
    
    logicalDevice.wait_idle();
    
    swapchain.clear(logicalDevice, nullptr);
    swapchain.create(logicalDevice, physicalDevice, surface, windowSize, nullptr);
    for (RenderPass& pass : renderPasses)
    {
        pass.clear_framebuffers(logicalDevice, nullptr);
        pass.clear_images(logicalDevice, nullptr);
        pass.create_attachments(physicalDevice, logicalDevice, swapchain, nullptr);
        pass.create_framebuffers(logicalDevice, swapchain, nullptr);
    }
}

Void Vulkan::reload_shaders(ShaderSet& shaderSet)
{
    Bool result = true;
    DynamicArray<Shader> pipelineShaders;
    pipelineShaders.reserve(shaderSet.shaderHandles.size());
    for (const Handle<ShaderVK> shaderHandle : shaderSet.shaderHandles)
    {
        Shader& shader = get_shader(shaderHandle);
        result &= shader.recreate(logicalDevice, nullptr);
        pipelineShaders.emplace_back(shader);
    }

    if (!result)
    {
        SPDLOG_ERROR("Failed to reload shaders.");
        return;
    }

    logicalDevice.wait_idle();

    const DescriptorPool& descriptorPool = get_descriptor_pool(shaderSet.descriptorPoolHandle);
    const RenderPass& renderPass = get_render_pass(shaderSet.renderPassHandle);

    Pipeline& pipeline = get_pipeline(shaderSet.pipelineHandle);
    pipeline.create_graphics_pipeline(descriptorPool,
                                      renderPass,
                                      pipelineShaders,
                                      logicalDevice,
                                      nullptr);
}

Void Vulkan::resize_image(const UVector2& newSize, Handle<Image> image)
{
    get_image(image).resize(physicalDevice, logicalDevice, newSize, nullptr);
}

Void Vulkan::transition_image_layout(Image& image, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageLayout newLayout)
{
    CommandBuffer commandBuffer;
    VkCommandBuffer buffer;
    begin_quick_commands(buffer);
    commandBuffer.set_buffer(buffer);

    commandBuffer.pipeline_image_barrier(image, sourceStage, destinationStage, newLayout);
    end_quick_commands(buffer);
}

Void Vulkan::generate_mipmaps(Image& image)
{
    VkCommandBuffer commandBuffer;
    begin_quick_commands(commandBuffer);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image.get_image();
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    const UVector2& size = image.get_size();
    Int32 mipWidth = size.x;
    Int32 mipHeight = size.y;
    const UInt32 mipLevels = image.get_mip_level();

    for (UInt32 i = 1; i < mipLevels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] =
        {
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1
        };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
                       image.get_image(),
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image.get_image(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1,
                       &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        if (mipWidth > 1)
        {
            mipWidth /= 2;
        }
        if (mipHeight > 1)
        {
            mipHeight /= 2;
        }

    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    image.set_current_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);

    end_quick_commands(commandBuffer);
}

Void Vulkan::copy_buffer_to_image(const Buffer& buffer, Image& image)
{
    VkCommandBuffer commandBuffer;
    begin_quick_commands(commandBuffer);

    const UVector2& size = image.get_size();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { size.x, size.y, 1 };

    vkCmdCopyBufferToImage(commandBuffer,
                           buffer.get_buffer(),
                           image.get_image(),
                           image.get_current_layout(),
                           1,
                           &region);

    end_quick_commands(commandBuffer);
}

Void Vulkan::copy_image_to_buffer(Buffer& buffer, Image& image)
{
    VkCommandBuffer commandBuffer;
    begin_quick_commands(commandBuffer);

    CommandBuffer bufferCommand;
    bufferCommand.set_buffer(commandBuffer);
    const VkImageLayout layout = image.get_current_layout();

    bufferCommand.pipeline_image_barrier(image,
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    const UVector2& size = image.get_size();
    VkBufferImageCopy region{};

    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { size.x, size.y, 1 };

    vkCmdCopyImageToBuffer(commandBuffer,
                           image.get_image(),
                           image.get_current_layout(),
                           buffer.get_buffer(),
                           1,
                           &region);

    bufferCommand.pipeline_image_barrier(image,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                         layout);

    end_quick_commands(commandBuffer);
}

Void Vulkan::copy_buffer(const Buffer& source, Buffer& destination)
{
    VkCommandBuffer commandBuffer;
    begin_quick_commands(commandBuffer);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = source.get_size();
    vkCmdCopyBuffer(commandBuffer, source.get_buffer(), destination.get_buffer(), 1, &copyRegion);

    end_quick_commands(commandBuffer);
}

Void Vulkan::begin_quick_commands(VkCommandBuffer& commandBuffer)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = get_command_pool(graphicsPool);
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(logicalDevice.get_device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

Void Vulkan::end_quick_commands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    logicalDevice.submit_graphics_queue({ submitInfo }, VK_NULL_HANDLE);
    logicalDevice.wait_graphics_queue_idle();

    vkFreeCommandBuffers(logicalDevice.get_device(),
                         get_command_pool(graphicsPool),
                         1, &commandBuffer);
}

Void Vulkan::shutdown()
{
    logicalDevice.wait_idle();
    SPDLOG_INFO("Wait until frame end...");

    for (Image& image : images)
    {
        image.clear(logicalDevice, nullptr);
    }
    images.clear();

    for (Buffer& buffer : buffers)
    {
        buffer.clear(logicalDevice, nullptr);
    }
    buffers.clear();

    for (VkCommandPool pool : commandPools)
    {
        vkDestroyCommandPool(logicalDevice.get_device(), pool, nullptr);
    }
    commandPools.clear();

    for (DescriptorPool& descriptorPool : descriptorPools)
    {
        descriptorPool.clear(logicalDevice, nullptr);
    }
    descriptorPools.clear();

    for (Pipeline& pipeline : pipelines)
    {
        pipeline.clear(logicalDevice, nullptr);
    }
    pipelines.clear();

    for (RenderPass& pass : renderPasses)
    {
        pass.clear(logicalDevice, nullptr);
    }
    renderPasses.clear();

    swapchain.clear(logicalDevice, nullptr);

    for (const VkSemaphore semaphore : semaphores)
    {
        vkDestroySemaphore(logicalDevice.get_device(), semaphore, nullptr);
    }
    semaphores.clear();

    for (VkFence fence : fences)
    {
        vkDestroyFence(logicalDevice.get_device(), fence, nullptr);
    }
    fences.clear();


    for (Shader& shader : shaders)
    {
        shader.clear(logicalDevice, nullptr);
    }
    shaders.clear();

    shaderSets.clear();

    logicalDevice.clear(nullptr);

    glslang::FinalizeProcess();

    if constexpr (DebugMessenger::ENABLE_VALIDATION_LAYERS)
    {
        debugMessenger.clear(instance, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}
