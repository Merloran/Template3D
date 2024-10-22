target_sources(Template3D PRIVATE
               vulkan_api.cpp vulkan_api.hpp
               Common/buffer_vk.cpp Common/buffer_vk.hpp
               Common/command_buffer.cpp Common/command_buffer.hpp
               Common/debug_messenger.cpp Common/debug_messenger.hpp
               Common/descriptor_pool.cpp Common/descriptor_pool.hpp
               Common/image_vk.cpp Common/image_vk.hpp
               Common/logical_device.cpp Common/logical_device.hpp
               Common/physical_device.cpp Common/physical_device.hpp
               Common/pipeline_vk.cpp Common/pipeline_vk.hpp
               Common/render_pass.cpp Common/render_pass.hpp
               Common/shader_set_vk.hpp
               Common/shader_vk.cpp Common/shader_vk.hpp
               Common/swapchain.cpp Common/swapchain.hpp)