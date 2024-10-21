#pragma once
#include "texture.hpp"

/** It's just set of textures */
template<typename API>
struct Material
{
    String name;
    Array<Handle<Texture<API>>, UInt64(ETextureType::Count)> textures;
    Float32	indexOfRefraction;
    Handle<typename API::ShaderSet> shaderSetHandle;

    Material()
        : indexOfRefraction(0.0f)
        , shaderSetHandle(Handle<typename API::ShaderSet>::NONE)
    {
        for (Handle<Texture<API>>& texture : textures)
        {
            texture = Handle<Texture<API>>::NONE;
        }
    }

    Handle<Texture<API>>& operator[](ETextureType type)
    {
        return textures[UInt64(type)];
    }

    Handle<Texture<API>> operator[](ETextureType type) const
    {
        return textures[UInt64(type)];
    }
};
