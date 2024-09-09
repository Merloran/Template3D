#pragma once
#include "texture.hpp"

/** It's just set of textures */
struct Material
{
	Array<Handle<Texture>, UInt64(ETextureType::Count)> textures;
	Float32	indexOfRefraction;
	String name;

	Material()
        : indexOfRefraction(0.0f)
    {
        for (Handle<Texture>& texture : textures)
        {
            texture = Handle<Texture>::NONE;
        }
    }

    Handle<Texture>& operator[](ETextureType type)
    {
		return textures[UInt64(type)];
	}

	Handle<Texture> operator[](ETextureType type) const
    {
		return textures[UInt64(type)];
	}
};
