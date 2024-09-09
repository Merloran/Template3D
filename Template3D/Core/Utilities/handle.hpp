#pragma once

template<typename Type>
struct Handle
{
    UInt64 id;
    static const Handle NONE;
};