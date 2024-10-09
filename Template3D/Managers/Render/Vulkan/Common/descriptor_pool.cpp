#include "descriptor_pool.hpp"

#include "logical_device.hpp"

#include <magic_enum.hpp>

//TODO: remove exceptions!

Void DescriptorPool::add_binding(const String& layoutName, UInt32 setNumber, UInt32 binding, VkDescriptorType descriptorType, UInt32 descriptorCount, VkShaderStageFlags stageFlags, VkDescriptorBindingFlags bindingFlags, VkDescriptorSetLayoutCreateFlags layoutFlags, VkDescriptorPoolCreateFlags poolFlags)
{
    if (setNumber + 1 > layoutData.size())
    {
        layoutData.resize(setNumber + 1);
    }

    DescriptorLayoutData &data = layoutData[setNumber];
    if (data.name.empty())
    {
        data.setNumber = setNumber;
        data.name = layoutName;
    }

    if (data.name != layoutName)
    {
        SPDLOG_ERROR("Layout binding with number: {}, name: {}, does not match: {}", setNumber, layoutName, data.name);
        return;
    }

    data.layoutFlags |= layoutFlags;
    this->poolFlags  |= poolFlags;
    if (bindingFlags != 0)
    {
        data.bindingFlags.push_back(bindingFlags);
    }
    VkDescriptorSetLayoutBinding& layoutBinding = data.bindings.emplace_back();
    layoutBinding.binding         = binding;
    layoutBinding.descriptorType  = descriptorType;
    layoutBinding.descriptorCount = descriptorCount;
    layoutBinding.stageFlags      = stageFlags;
}

Void DescriptorPool::create_layouts(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    create_empty(logicalDevice, allocator);
    
    for (UInt64 i = 0; i < layoutData.size(); ++i)
    {
        DescriptorLayoutData& data = layoutData[i];

        auto iterator = nameToIdLayoutData.find(data.name);
        if (iterator != nameToIdLayoutData.end())
        {
            SPDLOG_ERROR("Duplicated descriptor layout name: {}, descriptor layout skipped.", data.name);
            data.layout = VK_NULL_HANDLE;
            continue;
        }

        if (data.bindings.empty())
        {
            data.layout = VK_NULL_HANDLE;
            continue;
        }

        const Handle<DescriptorLayoutData> handle = { i };
        nameToIdLayoutData[data.name] = handle;

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
        bindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlags.pNext         = nullptr;
        bindingFlags.bindingCount  = UInt32(data.bindingFlags.size());
        bindingFlags.pBindingFlags = data.bindingFlags.data();

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = UInt32(data.bindings.size());
        layoutInfo.pBindings    = data.bindings.data();
        layoutInfo.pNext        = &bindingFlags;
        layoutInfo.flags        = data.layoutFlags;

        if (vkCreateDescriptorSetLayout(logicalDevice.get_device(), &layoutInfo, allocator, &data.layout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
}

Handle<DescriptorSetData> DescriptorPool::add_set(Handle<DescriptorLayoutData> layoutHandle, const DynamicArray<DescriptorResourceInfo>& resources, const String& name)
{
    if (nameToIdSetData.contains(name))
    {
        SPDLOG_ERROR("Failed to add descriptor set: {}. Descriptor set names must be unique.", name);
        return Handle<DescriptorSetData>::NONE;
    }

    const DescriptorLayoutData& layout = get_layout_data_by_handle(layoutHandle);
    if (!are_resources_compatible(layout, resources))
    {
        SPDLOG_ERROR("Fail to add descriptor set: {}", name);
        return Handle<DescriptorSetData>::NONE;
    }

    const Handle<DescriptorSetData> handle = { setData.size() };
    nameToIdSetData[name] = handle;

    DescriptorSetData& data = setData.emplace_back();
    data.name         = name;
    data.setNumber    = layout.setNumber;
    data.layoutHandle = layoutHandle;
    data.writes.resize(layout.bindings.size());
    data.resources = resources;

    for (UInt64 i = 0; i < data.writes.size(); ++i)
    {
        VkWriteDescriptorSet& write = data.writes[i];
	    write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	    write.pNext            = nullptr;
        write.dstArrayElement  = 0;
	    write.dstBinding       = layout.bindings[i].binding;
	    write.descriptorType   = layout.bindings[i].descriptorType;
        write.descriptorCount  = UInt32(data.resources[i].bufferInfos.size()
                               + data.resources[i].imageInfos.size()
                               + data.resources[i].texelBufferViews.size());
	    write.pBufferInfo      = data.resources[i].bufferInfos.data();
	    write.pImageInfo       = data.resources[i].imageInfos.data();
	    write.pTexelBufferView = data.resources[i].texelBufferViews.data();

	    VkDescriptorPoolSize& size = sizes.emplace_back();  
	    size.type            = write.descriptorType; 
        size.descriptorCount = write.descriptorCount;
    }
    return handle;
}

Void DescriptorPool::create_sets(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext         = nullptr;
    poolInfo.poolSizeCount = UInt32(sizes.size());
    poolInfo.pPoolSizes    = sizes.data();
    poolInfo.maxSets       = UInt32(setData.size());
    poolInfo.flags         = poolFlags;

    if (vkCreateDescriptorPool(logicalDevice.get_device(), &poolInfo, allocator, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    DynamicArray<UInt32> counts;
    DynamicArray<VkDescriptorSetLayout> layouts;
    DynamicArray<VkDescriptorSet> sets(setData.size(), VK_NULL_HANDLE);
    counts.reserve(setData.size());
    layouts.reserve(setData.size());
    for (const DescriptorSetData& set : setData)
    {
	    const UInt64 lastWriteIndex = set.writes.size() - 1;
        counts.push_back(set.writes[lastWriteIndex].descriptorCount);
        layouts.push_back(get_layout_data_by_handle(set.layoutHandle).layout);
    }


    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countAllocateInfo{};
    countAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
    countAllocateInfo.pNext              = nullptr;
    countAllocateInfo.descriptorSetCount = UInt32(counts.size());
    countAllocateInfo.pDescriptorCounts  = counts.data();

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext              = &countAllocateInfo;
    allocateInfo.descriptorPool     = pool;
    allocateInfo.descriptorSetCount = UInt32(setData.size());
    allocateInfo.pSetLayouts        = layouts.data();

    if (vkAllocateDescriptorSets(logicalDevice.get_device(), &allocateInfo, sets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (UInt64 i = 0; i < sets.size(); ++i)
    {
        DescriptorSetData& set = setData[i];
        set.set = sets[i];

        for (VkWriteDescriptorSet& write : set.writes)
        {
            write.dstSet = set.set;
        }

        vkUpdateDescriptorSets(logicalDevice.get_device(),
                               UInt32(set.writes.size()),
                               set.writes.data(),
                               0,
                               nullptr);
    }
}

Void DescriptorPool::update_set(const LogicalDevice& logicalDevice, const DescriptorResourceInfo& data, Handle<DescriptorSetData> handle, UInt32 arrayElement, UInt64 binding)
{
	DescriptorSetData& set             = get_set_data_by_handle(handle);
    VkWriteDescriptorSet& write        = set.writes[binding];
	const DescriptorLayoutData& layout = get_layout_data_by_handle(set.layoutHandle);

    write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext            = nullptr;
    write.dstArrayElement  = arrayElement;
    write.dstBinding       = layout.bindings[binding].binding;
    write.descriptorType   = layout.bindings[binding].descriptorType;
    write.descriptorCount  = UInt32(data.bufferInfos.size()
                           + data.imageInfos.size()
                           + data.texelBufferViews.size());
    write.pBufferInfo      = data.bufferInfos.data();
    write.pImageInfo       = data.imageInfos.data();
    write.pTexelBufferView = data.texelBufferViews.data();
    write.dstSet           = set.set;

    vkUpdateDescriptorSets(logicalDevice.get_device(),
                           1,
                           &write,
                           0,
                           nullptr);
}

Void DescriptorPool::set_push_constants(const DynamicArray<VkPushConstantRange>& pushConstants)
{
    this->pushConstants.reserve(pushConstants.size());

    UInt32 offset = 0;
    for (const VkPushConstantRange& constant : pushConstants)
    {
        VkPushConstantRange& range = this->pushConstants.emplace_back();
        range.size       = constant.size;
        range.offset     = offset;
        range.stageFlags = constant.stageFlags;
        offset += constant.size;
    }
}

Handle<DescriptorLayoutData> DescriptorPool::get_layout_data_handle_by_name(const String& name) const
{
    const auto& iterator = nameToIdLayoutData.find(name);
    if (iterator == nameToIdLayoutData.end())
    {
        SPDLOG_ERROR("Layout data handle {} not found, returned None.", name);
        return Handle<DescriptorLayoutData>::NONE;
    }

    return iterator->second;
}

DescriptorLayoutData& DescriptorPool::get_layout_data_by_name(const String& name)
{
    const auto& iterator = nameToIdLayoutData.find(name);
    if (iterator == nameToIdLayoutData.end() || iterator->second.id >= layoutData.size())
    {
        SPDLOG_ERROR("Layout data {} not found, returned default.", name);
        return layoutData[0];
    }

    return layoutData[iterator->second.id];
}

DescriptorLayoutData& DescriptorPool::get_layout_data_by_handle(const Handle<DescriptorLayoutData> handle)
{
    if (handle.id >= layoutData.size())
    {
        SPDLOG_ERROR("Layout data {} not found, returned default.", handle.id);
        return layoutData[0];
    }
    return layoutData[handle.id];
}

Handle<DescriptorSetData> DescriptorPool::get_set_data_handle_by_name(const String& name) const
{
    const auto& iterator = nameToIdSetData.find(name);
    if (iterator == nameToIdSetData.end())
    {
        SPDLOG_ERROR("Set data handle {} not found, returned None.", name);
        return Handle<DescriptorSetData>::NONE;
    }

    return iterator->second;
}

DescriptorSetData& DescriptorPool::get_set_data_by_name(const String& name)
{
    const auto& iterator = nameToIdSetData.find(name);
    if (iterator == nameToIdSetData.end() || iterator->second.id >= setData.size())
    {
        SPDLOG_ERROR("Set data {} not found, returned default.", name);
        return setData[0];
    }

    return setData[iterator->second.id];
}

DescriptorSetData& DescriptorPool::get_set_data_by_handle(const Handle<DescriptorSetData> handle)
{
    if (handle.id >= setData.size())
    {
        SPDLOG_ERROR("Set data {} not found, returned default.", handle.id);
        return setData[0];
    }
    return setData[handle.id];
}

DynamicArray<VkDescriptorSetLayout> DescriptorPool::get_layouts() const
{
    DynamicArray<VkDescriptorSetLayout> layouts;
    layouts.reserve(layoutData.size());
    for (const DescriptorLayoutData& data : layoutData)
    {
        if (data.layout == VK_NULL_HANDLE)
        {
            layouts.push_back(empty);
            continue;
        }
        layouts.push_back(data.layout);
    }
    return layouts;
}

Bool DescriptorPool::are_resources_compatible(const DescriptorLayoutData& layout, const DynamicArray<DescriptorResourceInfo>& resources) const
{
    if (layout.bindings.size() != resources.size())
    {
        SPDLOG_ERROR("Resources count does not match binding count in layout: {}", layout.name);
        return false;
    }

    for (UInt64 i = 0; i < layout.bindings.size(); ++i)
    {
        const VkDescriptorSetLayoutBinding& binding = layout.bindings[i];
        const DescriptorResourceInfo& resource = resources[i];
	    switch(binding.descriptorType)
	    {
	        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
	        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	        case VK_DESCRIPTOR_TYPE_SAMPLER:
	        {
                const UInt32 imagesCount = UInt32(resource.imageInfos.size());
	            if (resource.imageInfos.empty() || imagesCount > binding.descriptorCount)
	            {
                    SPDLOG_ERROR("Images {} size {} does not fit in binding descriptor count {}", i, imagesCount, binding.descriptorCount);
	                return false;
	            }
                break;
	        }

	        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
	        {
                const UInt32 buffersCount = UInt32(resource.texelBufferViews.size());
	            if (resource.texelBufferViews.empty() || buffersCount > binding.descriptorCount)
	            {
                    SPDLOG_ERROR("Texel views {} size {} does not fit in binding descriptor count {}", i, buffersCount, binding.descriptorCount);
	                return false;
	            }
                break;
	        }

	        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	        {
                const UInt32 buffersCount = UInt32(resource.texelBufferViews.size());
	            if (resource.bufferInfos.empty() || buffersCount > binding.descriptorCount)
	            {
                    SPDLOG_ERROR("Buffers {} size {} does not fit in binding descriptor count {}", i, buffersCount, binding.descriptorCount);
	                return false;
	            }
                break;
	        }

            default:
		    {
                SPDLOG_ERROR("Not supported descriptor type: {}", magic_enum::enum_name(binding.descriptorType));
                return false;
		    }
	    }
    }
    return true;
}

const DynamicArray<VkPushConstantRange>& DescriptorPool::get_push_constants() const
{
    return pushConstants;
}

Void DescriptorPool::create_empty(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 0;
    layoutInfo.pBindings    = nullptr;
    layoutInfo.pNext        = nullptr;
    layoutInfo.flags        = 0;

    if (vkCreateDescriptorSetLayout(logicalDevice.get_device(), &layoutInfo, allocator, &empty) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

Void DescriptorPool::clear(const LogicalDevice& logicalDevice, const VkAllocationCallbacks* allocator)
{
    vkDestroyDescriptorPool(logicalDevice.get_device(), pool, allocator);
    vkDestroyDescriptorSetLayout(logicalDevice.get_device(), empty, allocator);
    for (const DescriptorLayoutData& data : layoutData)
    {
        if (data.layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(logicalDevice.get_device(), data.layout, allocator);
        }
    }
}