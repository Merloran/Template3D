#pragma once

class ShaderVK;
class PipelineVK;
class DescriptorPool;
class RenderPass;

struct ShaderSetVK
{
    Handle<PipelineVK> pipelineHandle;
    Handle<RenderPass> renderPassHandle;
    Handle<DescriptorPool> descriptorPoolHandle;
    DynamicArray<Handle<ShaderVK>> shaderHandles;
};
