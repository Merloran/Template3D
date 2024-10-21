#pragma once
#include "vertex.hpp"

template<typename API>
struct Mesh 
{
    DynamicArray<Vertex> vertexes;
    DynamicArray<UInt32> indexes;
    String name;
    Handle<typename API::Buffer> vertexesHandle;
    Handle<typename API::Buffer> indexesHandle;

    Mesh() = default;
};
