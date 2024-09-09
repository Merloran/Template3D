#pragma once

struct Color
{
	UInt8 r, g, b, a;

	UInt8 operator[](UInt64 index) const
	{
		return reinterpret_cast<const UInt8*>(this)[index];
	}

	//HACK: MSVC still has bug with static inline members, and it's not working ;-;
	static const Color RED;
	static const Color GREEN;
	static const Color BLUE;
	static const Color INDIGO;
	static const Color BLACK;
	static const Color GRAY;
	static const Color WHITE;
};

const Color Color::RED	  = { .r = 255, .g =   0, .b =   0, .a = 255 };
const Color Color::GREEN  = { .r =   0, .g = 255, .b =   0, .a = 255 };
const Color Color::BLUE   = { .r =   0, .g =   0, .b = 255, .a = 255 };
const Color Color::INDIGO = { .r = 128, .g = 128, .b = 255, .a = 255 };
const Color Color::BLACK  = { .r =   0, .g =   0, .b =   0, .a = 255 };
const Color Color::GRAY   = { .r = 128, .g = 128, .b = 128, .a = 255 };
const Color Color::WHITE  = { .r = 255, .g = 255, .b = 255, .a = 255 };