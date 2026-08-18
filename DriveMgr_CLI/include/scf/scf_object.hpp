#pragma once


namespace scf {

template<typename T, typename... Args>
T* create_at(void* storage, Args&&... args) noexcept
{
    return ::new (storage) T(static_cast<Args&&>(args)...);
}

template<typename T>
void destroy_at(T* object) noexcept
{
    if (object) {
        object->~T();
    }
}

} 