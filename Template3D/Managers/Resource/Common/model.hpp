#pragma once

template<typename API>
struct Mesh;
template<typename API>
struct Material;

/** It's just set of meshes and materials */
template<typename API>
struct Model
{
    DynamicArray<Handle<Mesh<API>>> meshes;
    DynamicArray<Handle<Material<API>>> materials;
    String directory;
    String name;

    Model() = default;
};