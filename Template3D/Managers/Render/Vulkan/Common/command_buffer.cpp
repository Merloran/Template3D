#include "command_buffer.hpp"

#include <magic_enum.hpp>

#include "logical_device.hpp"
#include "pipeline_vk.hpp"
#include "render_pass.hpp"
#include "swapchain.hpp"
#include "buffer.hpp"
#include "image.hpp"

Void CommandBuffer::begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritanceInfo, Void* next) const
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = next;
    beginInfo.flags            = flags;
    beginInfo.pInheritanceInfo = inheritanceInfo;

    const VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Command buffer: {}, begin failed with result: {}.", name, magic_enum::enum_name(result));
    }
}

Void CommandBuffer::begin_render_pass(const RenderPass& renderPass, const Swapchain& swapchain, UInt64 imageIndex, VkSubpassContents subpassContents) const
{
    VkRenderPassBeginInfo renderPassInfo{};
    const DynamicArray<VkClearValue>& clearValues = renderPass.get_clear_values();
    const UVector2& extent = swapchain.get_extent();
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext             = nullptr; //TODO: think of it
    renderPassInfo.renderPass        = renderPass.get_render_pass();
    renderPassInfo.framebuffer       = renderPass.get_framebuffer(imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = VkExtent2D{ extent.x, extent.y };
    renderPassInfo.clearValueCount   = UInt32(clearValues.size());
    renderPassInfo.pClearValues      = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, subpassContents);
}

Void CommandBuffer::bind_pipeline(const PipelineVK& pipeline) const
{
    vkCmdBindPipeline(commandBuffer, pipeline.get_bind_point(), pipeline.get_pipeline());
}

Void CommandBuffer::bind_descriptor_set(const PipelineVK& pipeline, VkDescriptorSet set, UInt32 setNumber, const DynamicArray<UInt32> &dynamicOffsets) const
{
    vkCmdBindDescriptorSets(commandBuffer,
                            pipeline.get_bind_point(),
                            pipeline.get_layout(),
                            setNumber,
                            1,
                            &set,
                            UInt32(dynamicOffsets.size()),
                            dynamicOffsets.data());
}

Void CommandBuffer::bind_index_buffer(VkBuffer indexBuffer, UInt64 offset, VkIndexType type) const
{
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, offset, type);
}

Void CommandBuffer::draw_indexed(UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const
{
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

Void CommandBuffer::dispatch(const UVector3& groupCount) const
{
    vkCmdDispatch(commandBuffer, groupCount.x, groupCount.y, groupCount.z);
}

Void CommandBuffer::set_constants(const PipelineVK& pipeline, VkShaderStageFlags stageFlags, UInt32 offset, UInt32 size, Void* data) const
{
    vkCmdPushConstants(commandBuffer, pipeline.get_layout(), stageFlags, offset, size, data);
}

Void CommandBuffer::set_viewports(UInt32 firstViewport, const DynamicArray<VkViewport>& viewports) const
{
    vkCmdSetViewport(commandBuffer, firstViewport, UInt32(viewports.size()), viewports.data());
}

Void CommandBuffer::set_viewport(UInt32 firstViewport, const FVector2& position, const FVector2& size, const FVector2& depthBounds) const
{
    VkViewport viewport{};
    viewport.x        = position.x;
    viewport.y        = position.y;
    viewport.width    = size.x;
    viewport.height   = size.y;
    viewport.minDepth = depthBounds.x;
    viewport.maxDepth = depthBounds.y;
    vkCmdSetViewport(commandBuffer, firstViewport, 1, &viewport);
}

Void CommandBuffer::set_scissors(UInt32 firstScissor, const DynamicArray<VkRect2D>& scissors) const
{
    vkCmdSetScissor(commandBuffer, firstScissor, UInt32(scissors.size()), scissors.data());
}

Void CommandBuffer::set_scissor(UInt32 firstScissor, const IVector2& position, const UVector2& size) const
{
    VkRect2D scissor{};
    scissor.offset.x      = position.x;
    scissor.offset.y      = position.y;
    scissor.extent.width  = size.x;
    scissor.extent.height = size.y;
    vkCmdSetScissor(commandBuffer, firstScissor, 1, &scissor);
}

Void CommandBuffer::pipeline_barrier(VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkDependencyFlags dependencies, const DynamicArray<VkMemoryBarrier>& memoryBarriers, const DynamicArray<VkBufferMemoryBarrier>& bufferBarriers, const DynamicArray<VkImageMemoryBarrier>& imageBarriers) const
{
    vkCmdPipelineBarrier(commandBuffer,
                         sourceStage,
                         destinationStage,
                         dependencies, 
                         UInt32(memoryBarriers.size()), 
                         memoryBarriers.data(), 
                         UInt32(bufferBarriers.size()),
                         bufferBarriers.data(), 
                         UInt32(imageBarriers.size()),
                         imageBarriers.data());
}

Void CommandBuffer::pipeline_memory_barrier(VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkDependencyFlags dependencies, VkAccessFlags sourceAccess, VkAccessFlags destinationAccess, Void* next) const
{
    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext         = next;
    memoryBarrier.srcAccessMask = sourceAccess;
    memoryBarrier.dstAccessMask = destinationAccess;

    vkCmdPipelineBarrier(commandBuffer,
                         sourceStage,
                         destinationStage,
                         dependencies,
                         1,
                         &memoryBarrier,
                         0,
                         nullptr,
                         0,
                         nullptr);
}

Void CommandBuffer::pipeline_buffer_barrier(VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkDependencyFlags dependencies, VkAccessFlags sourceAccess, VkAccessFlags destinationAccess, UInt32 sourceQueueIndex, UInt32 destinationQueueIndex, const Buffer& buffer, UInt64 offset, Void* next) const
{
    VkBufferMemoryBarrier bufferBarrier{};
    bufferBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferBarrier.pNext               = next;
    bufferBarrier.srcAccessMask       = sourceAccess;
    bufferBarrier.dstAccessMask       = destinationAccess;
    bufferBarrier.srcQueueFamilyIndex = sourceQueueIndex;
    bufferBarrier.dstQueueFamilyIndex = destinationQueueIndex;
    bufferBarrier.buffer              = buffer.get_buffer();
    bufferBarrier.offset              = offset;
    bufferBarrier.size                = buffer.get_size();

    vkCmdPipelineBarrier(commandBuffer,
                         sourceStage,
                         destinationStage,
                         dependencies,
                         0,
                         nullptr,
                         1,
                         &bufferBarrier,
                         0,
                         nullptr);
}

Void CommandBuffer::pipeline_image_barrier(VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkDependencyFlags dependencies, VkAccessFlags sourceAccess, VkAccessFlags destinationAccess, UInt32 sourceQueueIndex, UInt32 destinationQueueIndex, Vulkan::Image& image, VkImageLayout newLayout, Void* next) const
{
    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext                           = next;
    barrier.srcAccessMask                   = sourceAccess;
    barrier.dstAccessMask                   = destinationAccess;
    barrier.oldLayout                       = image.get_current_layout();
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = sourceQueueIndex;
    barrier.dstQueueFamilyIndex             = destinationQueueIndex;
    barrier.image                           = image.get_image();
    barrier.subresourceRange.aspectMask     = image.get_aspect_flags();
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = image.get_mip_level();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    vkCmdPipelineBarrier(commandBuffer,
                         sourceStage,
                         destinationStage,
                         dependencies,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);

    image.set_current_layout(newLayout);
}

Void CommandBuffer::pipeline_image_barrier(Vulkan::Image& image, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageLayout newLayout) const
{
    const VkImageLayout oldLayout = image.get_current_layout();

    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext                           = nullptr;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image.get_image();
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = image.get_mip_level();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        VkFormat format = image.get_format();
        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    switch (oldLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
	    {
		    barrier.srcAccessMask = 0;
    		break;
	    }
        case VK_IMAGE_LAYOUT_GENERAL:
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
            break;
        }
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
		{
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		}
		default:
		{
			SPDLOG_ERROR("Not supported old layout transition: {}", magic_enum::enum_name(oldLayout));
			return;
		}
    }
    
    switch (newLayout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		{
		    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		    break;
		}
	    case VK_IMAGE_LAYOUT_GENERAL:
		{
	        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	        break;
		}
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		{
		    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		    break;
		}
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		{
		    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		    break;
		}
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		{
		    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		    break;
		}
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		{
		    if (barrier.srcAccessMask == 0) 
            {
			    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		    }

		    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		    break;
		}
		default:
	    {
		    SPDLOG_ERROR("Not supported new layout transition: {}", magic_enum::enum_name(newLayout));
    		return;
	    }
    }
    image.set_current_layout(newLayout);

    vkCmdPipelineBarrier(commandBuffer,
                         sourceStage,
                         destinationStage,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);
}

Void CommandBuffer::end_render_pass() const
{
    vkCmdEndRenderPass(commandBuffer);
}

Void CommandBuffer::end() const
{
    const VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Command buffer: {}, end failed with result: {}.", name, magic_enum::enum_name(result));
    }
}

Void CommandBuffer::reset(VkCommandBufferResetFlags flags) const
{
    const VkResult result = vkResetCommandBuffer(commandBuffer, flags);
    if (result != VK_SUCCESS)
    {
        SPDLOG_ERROR("Command buffer: {}, reset failed with result: {}.", name, magic_enum::enum_name(result));
    }
}

const VkCommandBuffer& CommandBuffer::get_buffer() const
{
	return commandBuffer;
}

const String& CommandBuffer::get_name() const
{
    return name;
}

Void CommandBuffer::set_name(const String& name)
{
    this->name = name;
}

Void CommandBuffer::set_buffer(VkCommandBuffer buffer)
{
    this->commandBuffer = buffer;
}