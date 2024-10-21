#pragma once
#include "../../Common/pipeline_type.hpp"

class DescriptorPool;
class RenderPass;
class ShaderGL;
class LogicalDevice;

class PipelineGL //TODO: set error check only for debug mode
{
private:
	UInt32 pipeline;
	EPipelineType type;

public:
	Bool create_graphics_pipeline(const DynamicArray<ShaderGL>& shaders);

	Bool create_compute_pipeline(const ShaderGL& shader);

	Bool recreate_pipeline(const DynamicArray<ShaderGL>& shaders);

    Void set_bool  (const String& name, Bool value);
    Void set_int   (const String& name, Int32 value);
    Void set_float (const String& name, Float32 value);
    Void set_vec2  (const String& name, Float32 x, Float32 y);
    Void set_vec2  (const String& name, const FVector2 &vector);
    Void set_ivec2 (const String& name, Int32 x, Int32 y);
    Void set_ivec2 (const String& name, const IVector2 &vector);
    Void set_vec3  (const String& name, Float32 x, Float32 y, Float32 z);
    Void set_vec3  (const String& name, const FVector3 &vector);
    Void set_vec4  (const String& name, Float32 x, Float32 y, Float32 z, Float32 w);
    Void set_vec4  (const String& name, const FVector4 &vector);
    Void set_mat4  (const String& name, const FMatrix4 &value);
    Void set_block (const String& name, UInt32 number);

	static Void bind_uniform_buffer(UInt32 uniformBufferObject, UInt32 offset, UInt32 size, const Float32* data);

	Void bind() const;

	[[nodiscard]]
	EPipelineType get_type() const;
	[[nodiscard]]
	UInt32 get_pipeline() const;

	Void clear();

private:
	Bool has_compilation_errors() const;

	static inline UInt32 activePipeline = 0U;
};
