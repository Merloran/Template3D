#pragma once

template<typename Type>
struct Handle;

/** It's just set of meshes and materials */
struct Model
{
	std::vector<Handle<struct Mesh>> meshes;
	std::vector<Handle<struct Material>> materials;
	std::string directory;
};