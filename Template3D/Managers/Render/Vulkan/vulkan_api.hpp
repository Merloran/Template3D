#pragma once
#include "Common/debug_messenger.hpp"
#include "Common/physical_device.hpp"
#include "Common/logical_device.hpp"
#include "Common/swapchain.hpp"
#include "Common/render_pass.hpp"
#include "Common/descriptor_pool.hpp"
#include "Common/pipeline_vk.hpp"
#include "Common/image.hpp"
#include "Common/buffer.hpp"

#include <vulkan/vulkan.hpp>


struct ShaderSetVK;
class Camera;
template<typename API>
struct Texture;
template<typename API>
struct Model;
template<typename API>
struct Mesh;
class CommandBuffer;
template<typename Type>
struct Handle;
class ShaderVK;
enum class EShaderType : UInt8;


struct UniformBufferObject
{
    FMatrix4 viewProjection;
    Float32 time;
};

struct VertexConstants
{
    FMatrix4 model;
};

struct FragmentConstants
{
    UInt32 albedoId;
    UInt32 metalnessId;
    UInt32 roughnessId;
    UInt32 emissionId;
};

class Vulkan
{
public:
    const String SHADERS_PATH = "Resources/Shaders/";
    const String COMPILED_SHADER_EXTENSION = ".spv";
    const String GLSL_COMPILER_PATH = "D:/VulkanSDK/Bin/glslc.exe";
    using Buffer = Buffer;
    using Image = Image;
    using Shader = ShaderVK;
    using Pipeline = PipelineVK;
    using ShaderSet = ShaderSetVK;

private:
    VkInstance instance;
    DebugMessenger debugMessenger;
    VkSurfaceKHR surface;

    PhysicalDevice physicalDevice;
    LogicalDevice logicalDevice;
    Swapchain swapchain;
    Handle<RenderPass> rasterizePass;
    DynamicArray<RenderPass> renderPasses;

    DescriptorPool descriptorPool;
    Pipeline graphicsPipeline;

    DynamicArray<Shader> shaders;
    HashMap<String, Handle<Shader>> shadersNameMap;
    DynamicArray<Pipeline> pipelines;
    DynamicArray<ShaderSet> shaderSets;

    Handle<VkCommandPool> graphicsPool;
    DynamicArray<VkCommandPool> commandPools;

    DynamicArray<CommandBuffer> commandBuffers;
    HashMap<String, Handle<CommandBuffer>> commandBuffersNameMap;

    Handle<Buffer> uniformBufferHandle;
    DynamicArray<Buffer> buffers;
    DynamicArray<Image> images;

    Handle<VkFence> inFlightFence;
    DynamicArray<VkFence> fences;
    HashMap<String, Handle<VkFence>> fencesNameMap;

    Handle<VkSemaphore> renderFinished;
    DynamicArray<VkSemaphore> semaphores;
    HashMap<String, Handle<VkSemaphore>> semaphoresNameMap;

    Bool isFrameEven;

public:
    Void startup();

    [[nodiscard]]
    VkSurfaceKHR get_surface() const;
    [[nodiscard]]
    const PhysicalDevice& get_physical_device() const;
    [[nodiscard]]
    const LogicalDevice& get_logical_device() const;
    Swapchain& get_swapchain();
    DescriptorPool& get_pool();

    [[nodiscard]]
    const Handle<Shader>& get_shader_handle(const String& name)  const;
    Shader& get_shader(const String& name);
    Shader& get_shader(const Handle<Shader> handle);
    Pipeline& get_pipeline(const Handle<Pipeline> handle);
    ShaderSet& get_shader_set(const Handle<ShaderSet> handle);

    [[nodiscard]]
    const Handle<CommandBuffer>& get_command_buffer_handle(const String& name)  const;
    CommandBuffer& get_command_buffer(const String& name);
    CommandBuffer& get_command_buffer(const Handle<CommandBuffer> handle);

    [[nodiscard]]
    const Handle<VkSemaphore>& get_semaphore_handle(const String& name)  const;
    VkSemaphore get_semaphore(const String& name);
    VkSemaphore get_semaphore(const Handle<VkSemaphore> handle);

    [[nodiscard]]
    const Handle<VkFence>& get_fence_handle(const String& name)  const;
    VkFence get_fence(const String& name);
    VkFence get_fence(const Handle<VkFence> handle);

    Image& get_image(const Handle<Image> handle);
    Buffer& get_buffer(const Handle<Buffer> handle);
    VkCommandPool get_command_pool(const Handle<VkCommandPool> handle);
    RenderPass& get_render_pass(const Handle<RenderPass> handle);


    Handle<Shader> load_shader(const String& filePath, const EShaderType shaderType, const String& functionName = "main");
    Void generate_mesh_buffers(DynamicArray<Mesh<Vulkan>>& meshes);
    Void create_mesh_buffers(Mesh<Vulkan>& mesh);
    Void generate_texture_images(DynamicArray<Texture<Vulkan>>& textures);
    Void create_texture_image(Texture<Vulkan>& texture, UInt32 mipLevels = 1);
    Void load_pixels_from_image(Texture<Vulkan>& texture);
    Handle<Image> create_image(const UVector2& size,
                               VkFormat format,
                               VkImageUsageFlags usage,
                               VkImageTiling tiling,
                               UInt32 mipLevels = 1);

    template <typename Type>
    Handle<Buffer> create_dynamic_buffer(VkBufferUsageFlagBits usage,
                                        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                                                                         | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    {
        const VkDeviceSize bufferSize = sizeof(Type);
        const Handle<Buffer> handle = { buffers.size() };
        Buffer& buffer = buffers.emplace_back();

        buffer.create(physicalDevice,
                      logicalDevice,
                      bufferSize,
                      usage,
                      properties,
                      nullptr);
        vkMapMemory(logicalDevice.get_device(), buffer.get_memory(), 0, bufferSize, 0, buffer.get_mapped_memory());

        return handle;
    }

    template<typename Type>
    Handle<Buffer> create_static_buffer(const DynamicArray<Type>& data,
                                       VkBufferUsageFlagBits usage,
                                       VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        const UInt64 bufferSize = sizeof(Type) * data.size();

        Buffer stagingBuffer{};
        stagingBuffer.create(physicalDevice,
                             logicalDevice,
                             bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             nullptr);

        Void* addressToGPU;
        vkMapMemory(logicalDevice.get_device(), stagingBuffer.get_memory(), 0, bufferSize, 0, &addressToGPU);
        memcpy(addressToGPU, data.data(), bufferSize);
        vkUnmapMemory(logicalDevice.get_device(), stagingBuffer.get_memory());

        const Handle<Buffer> handle = { buffers.size() };
        Buffer& buffer = buffers.emplace_back();

        buffer.create(physicalDevice,
                      logicalDevice,
                      bufferSize,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
                      properties,
                      nullptr);

        copy_buffer(stagingBuffer, buffer);

        stagingBuffer.clear(logicalDevice, nullptr);

        return handle;
    }

    Handle<RenderPass> create_render_pass(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
                                          Bool depthTest = true,
                                          VkAttachmentLoadOp loadOperation = VK_ATTACHMENT_LOAD_OP_CLEAR);

    Handle<VkCommandPool> create_command_pool(VkCommandPoolCreateFlagBits flags);
    Void create_command_buffers(Handle<VkCommandPool> handle, VkCommandBufferLevel level, const DynamicArray<String>& names);
    Void create_command_buffers(VkCommandPool pool, VkCommandBufferLevel level, const DynamicArray<String>& names);

    Handle<VkSemaphore> create_semaphore(const String& name);
    Handle<VkFence> create_fence(const String& name, VkFenceCreateFlags flags = 0);


    Void setup_graphics_descriptors(const DynamicArray<Texture<Vulkan>>& textures);
    Void recreate_swapchain();
    Void reload_shaders();
    Void resize_image(const UVector2& newSize, Handle<Image> image);
    Void transition_image_layout(Image& image,
                                 VkPipelineStageFlags sourceStage,
                                 VkPipelineStageFlags destinationStage,
                                 VkImageLayout newLayout);
    Void shutdown();

private:
    Void create_graphics_descriptors();
    Void create_vulkan_instance();
    Void create_surface();
    DynamicArray<const Char*> get_required_extensions();

    Void generate_mipmaps(Image& image);
    Void copy_buffer_to_image(const Buffer& buffer, Image& image);
    Void copy_image_to_buffer(Buffer& buffer, Image& image);
    Void copy_buffer(const Buffer& source, Buffer& destination);
    Void begin_quick_commands(VkCommandBuffer& commandBuffer);
    Void end_quick_commands(VkCommandBuffer commandBuffer);
};