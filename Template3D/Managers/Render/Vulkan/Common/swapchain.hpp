#pragma once
#include <vulkan/vulkan.hpp>

class RenderPass;
class PhysicalDevice;
class LogicalDevice;

class Swapchain
{
private:
    VkSwapchainKHR            swapchain;
    DynamicArray<VkImage>     images;
    DynamicArray<VkImageView> imageViews;
    VkFormat                  imageFormat;
    UVector2                  extent;
    UInt32                    imageIndex;

public:
    Void create(const LogicalDevice& logicalDevice,
                const PhysicalDevice& physicalDevice,
                const VkSurfaceKHR& surface,
                const IVector2& framebufferSize,
                const VkAllocationCallbacks* allocator);

    const VkSwapchainKHR &get_swapchain() const;
    VkFormat get_image_format() const;
    const UVector2 &get_extent() const;
    const DynamicArray<VkImageView> &get_image_views() const;
    const UInt32 &get_image_index() const;
    Void set_image_index(UInt32 imageIndex);

    Void clear(const LogicalDevice& device, const VkAllocationCallbacks* allocator);

private:
    Void create_image_views(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator);

    VkSurfaceFormatKHR choose_swap_surface_format(const DynamicArray<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choose_swap_present_mode(const DynamicArray<VkPresentModeKHR>& availablePresentModes);
    UVector2 choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const IVector2& framebufferSize);
};
