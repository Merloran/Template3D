#pragma once

class ShaderGL;
class PipelineGL;

struct ShaderSetGL
{
    Handle<PipelineGL> pipelineHandle;
    DynamicArray<Handle<ShaderGL>> shaderHandles;
};
