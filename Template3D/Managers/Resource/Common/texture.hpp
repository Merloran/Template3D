#pragma once

enum class ETextureType : Int16
{
	None = 0,

	Albedo,
	Normal,
	Roughness,
	Metalness,
	AmbientOcclusion,
	Emission,
	Height,
	Opacity,
	HDR,

	RM,
	RMAO,

	Count,
};

struct Texture 
{
	IVector2 size;
	String name;
	UInt8* data; //TODO: change it to DynamicArray after changing image loading library
	Int32 channels; //TODO: change it to UInt8 after changing image loading library
	ETextureType type;

	Texture()
        : size()
        , data(nullptr)
        , channels(0)
        , type(ETextureType::None)
    {}
};