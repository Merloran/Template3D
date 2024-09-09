#pragma once
#include "vertex.hpp"

struct Mesh 
{
	DynamicArray<Vertex> vertexes;
	DynamicArray<UInt32> indexes;
	String name;

	Mesh() = default;
};
