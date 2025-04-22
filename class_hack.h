#pragma once

// call function
template <typename FuncType>
__forceinline static FuncType __call(void* ptr, int offset)
{
    return (FuncType)((*(int**)ptr)[offset / 4]);
}

// get property
template <typename PropType>
__forceinline static PropType& __property(void* ptr, int offset)
{
    return *reinterpret_cast<PropType*>(reinterpret_cast<uintptr_t*>(ptr) + offset * 4);
}
