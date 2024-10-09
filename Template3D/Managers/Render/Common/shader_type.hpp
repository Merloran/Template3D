#pragma once

enum class EShaderType : UInt8
{
	None = 0U,
	Vertex,
	Geometry,
	Fragment,
	Compute,
	Count
};