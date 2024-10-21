#pragma once

template<typename Type>
struct Handle
{
    UInt64 id;
    static const Handle NONE;

    Bool operator==(Handle<Type> other)
    {
        return this->id == other.id;
    }

    Bool operator!=(Handle<Type> other)
    {
        return this->id != other.id;
    }
};