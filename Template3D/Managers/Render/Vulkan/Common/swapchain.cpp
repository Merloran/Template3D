#include "swapchain.hpp"

#include "physical_device.hpp"
#include "logical_device.hpp"
#include "image_vk.hpp"

#include <magic_enum.hpp>

Void Swapchain::create(const LogicalDevice& logicalDevice, const PhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const IVector2& framebufferSize, const VkAllocationCallbacks* allocator)
{
    const VkSurfaceCapabilitiesKHR& capabilities = physicalDevice.get_capabilities(surface);
    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(physicalDevice.get_formats(surface));
    VkPresentModeKHR   presentMode   = choose_swap_present_mode(physicalDevice.get_present_modes(surface));
    extent                           = choose_swap_extent(capabilities, framebufferSize);
    UInt32             imageCount    = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0)
    {
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR createInfo{};
    Array<UInt32, 2> queueFamilyIndices = 
    {
        physicalDevice.get_graphics_family_index(),
        physicalDevice.get_present_family_index()
    };

    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = VkExtent2D{ extent.x, extent.y };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (physicalDevice.get_graphics_family_index() != physicalDevice.get_present_family_index())
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = UInt32(queueFamilyIndices.size());
        createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }
    createInfo.preTransform     = capabilities.currentTransform;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = presentMode;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(logicalDevice.get_device(), &createInfo, allocator, &swapchain);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Swapchain creation failed with: {}", magic_enum::enum_name(result));
        return;
    }
    vkGetSwapchainImagesKHR(logicalDevice.get_device(), swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice.get_device(), swapchain, &imageCount, images.data());
    imageFormat = surfaceFormat.format;

    create_image_views(logicalDevice, allocator);
}

const VkSwapchainKHR& Swapchain::get_swapchain() const
{
    return swapchain;
}

VkFormat Swapchain::get_image_format() const
{
    return imageFormat;
}

const UVector2& Swapchain::get_extent() const
{
    return extent;
}

const DynamicArray<VkImageView>& Swapchain::get_image_views() const
{
    return imageViews;
}

const UInt32 &Swapchain::get_image_index() const
{
    return imageIndex;
}

Void Swapchain::set_image_index(UInt32 imageIndex)
{
    this->imageIndex = imageIndex;
}

Void Swapchain::create_image_views(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    imageViews.resize(images.size());

    for (UInt64 i = 0; i < imageViews.size(); ++i)
    {
        imageViews[i] = ImageVK::s_create_view(logicalDevice,
                                             images[i],
                                             imageFormat, 
                                             VK_IMAGE_ASPECT_COLOR_BIT, 
                                             1, 
                                             allocator);
    }
}

VkSurfaceFormatKHR Swapchain::choose_swap_surface_format(const DynamicArray<VkSurfaceFormatKHR>& availableFormats)
{
    for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
    { //TODO: Make this as parameter to change
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Swapchain::choose_swap_present_mode(const DynamicArray<VkPresentModeKHR>& availablePresentModes)
{
    for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

UVector2 Swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const IVector2& framebufferSize)
{
    if (capabilities.currentExtent.width != Limits<UInt32>::max())
    {
        return UVector2{ capabilities.currentExtent.width, capabilities.currentExtent.height };
    } else {
        UVector2 actualExtent = framebufferSize;
    
        actualExtent.x = std::clamp(actualExtent.x,
                                    capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
        actualExtent.y = std::clamp(actualExtent.y,
                                    capabilities.minImageExtent.height,
                                    capabilities.maxImageExtent.height);
    
        return actualExtent;
    }
    return {};
}

Void Swapchain::clear(const LogicalDevice& device, const VkAllocationCallbacks* allocator)
{
    for (VkImageView imageView : imageViews)
    {
        vkDestroyImageView(device.get_device(), imageView, allocator);
    }

    vkDestroySwapchainKHR(device.get_device(), swapchain, allocator);
}