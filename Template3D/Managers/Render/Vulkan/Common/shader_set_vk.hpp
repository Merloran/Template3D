#pragma once

class ShaderVK;
class PipelineVK;

struct ShaderSetVK
{
    Handle<PipelineVK> pipelineHandle;
    DynamicArray<Handle<ShaderVK>> shaderHandles;
};
