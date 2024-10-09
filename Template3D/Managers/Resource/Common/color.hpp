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