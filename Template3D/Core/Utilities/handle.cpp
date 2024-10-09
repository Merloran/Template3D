#include "handle.hpp"

#include "../../Managers/Display/display_manager.hpp"

#include "../../Managers/Resource/Common/texture.hpp"
#include "../../Managers/Resource/Common/mesh.hpp"
#include "../../Managers/Resource/Common/model.hpp"
#include "../../Managers/Resource/Common/material.hpp"

#include "../../Managers/Render/Vulkan/vulkan_api.hpp"
#include "../../Managers/Render/OpenGL/opengl_api.hpp"

const Handle<DisplayManager::Window> Handle<DisplayManager::Window>::NONE = { UInt64(-1) };

const Handle<Model<Vulkan>> Handle<Model<Vulkan>>::NONE               = { UInt64(-1) };
const Handle<Mesh<Vulkan>> Handle<Mesh<Vulkan>>::NONE                 = { UInt64(-1) };
const Handle<Material<Vulkan>> Handle<Material<Vulkan>>::NONE         = { UInt64(-1) };
const Handle<Texture<Vulkan>> Handle<Texture<Vulkan>>::NONE           = { UInt64(-1) };
const Handle<Vulkan::Image> Handle<Vulkan::Image>::NONE               = { UInt64(-1) };
const Handle<Vulkan::Buffer> Handle<Vulkan::Buffer>::NONE             = { UInt64(-1) };
const Handle<Vulkan::Shader> Handle<Vulkan::Shader>::NONE             = { UInt64(-1) };
const Handle<Vulkan::Pipeline> Handle<Vulkan::Pipeline>::NONE         = { UInt64(-1) };
const Handle<Vulkan::ShaderSet> Handle<Vulkan::ShaderSet>::NONE       = { UInt64(-1) };
const Handle<DescriptorSetData> Handle<DescriptorSetData>::NONE       = { UInt64(-1) };
const Handle<DescriptorLayoutData> Handle<DescriptorLayoutData>::NONE = { UInt64(-1) };
const Handle<CommandBuffer> Handle<CommandBuffer>::NONE               = { UInt64(-1) };
const Handle<VkCommandPool> Handle<VkCommandPool>::NONE               = { UInt64(-1) };
const Handle<VkSemaphore> Handle<VkSemaphore>::NONE                   = { UInt64(-1) };
const Handle<VkFence> Handle<VkFence>::NONE                           = { UInt64(-1) };


const Handle<Model<OpenGL>> Handle<Model<OpenGL>>::NONE         = { UInt64(-1) };
const Handle<Mesh<OpenGL>> Handle<Mesh<OpenGL>>::NONE           = { UInt64(-1) };
const Handle<Material<OpenGL>> Handle<Material<OpenGL>>::NONE   = { UInt64(-1) };
const Handle<Texture<OpenGL>> Handle<Texture<OpenGL>>::NONE     = { UInt64(-1) };
const Handle<OpenGL::Shader> Handle<OpenGL::Shader>::NONE       = { UInt64(-1) };
const Handle<OpenGL::Pipeline> Handle<OpenGL::Pipeline>::NONE   = { UInt64(-1) };
const Handle<OpenGL::ShaderSet> Handle<OpenGL::ShaderSet>::NONE = { UInt64(-1) };
const Handle<UInt32> Handle<UInt32>::NONE                       = { UInt64(-1) };