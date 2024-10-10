#pragma once
#include <vulkan/vulkan.hpp>

class PhysicalDevice;
class LogicalDevice;

class BufferVK
{
private:
    VkBuffer buffer;
    VkDeviceMemory memory;
    Void* mappedMemory;
    UInt64 size;

public:
    Void create(const PhysicalDevice& physicalDevice,
                const LogicalDevice& logicalDevice,
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties,
                const VkAllocationCallbacks* allocator);

    VkBuffer get_buffer() const;
    VkDeviceMemory get_memory() const;
    Void** get_mapped_memory();
    UInt64 get_size() const;

    template <typename Type>
    Void update_dynamic_buffer(const Type& data)
    {
        memcpy(mappedMemory, &data, sizeof(Type));
    }

    Void clear(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator);
};

