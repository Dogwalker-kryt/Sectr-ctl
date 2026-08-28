#pragma once

#include <stddef.h>
#include "scf_exception.hpp"
#include "scf_type_traits.hpp"
#include "config.hpp"

namespace scf {

template<typename T>
class span {
private:
    T* ptr_ = nullptr;
    size_t size_ = 0;

public:
    span() = default;

    span(T* ptr, size_t size)
        : ptr_(ptr), size_(size) {}

    span(T* first, T* last)
        : ptr_(first), size_(static_cast<size_t>(last - first)) {}

    // C-style array
    template<size_t N>
    span(T (&array)[N])
        : ptr_(array), size_(N) {}

    // scf::array
    template<size_t N>
    span(scf::array<T, N>& array)
        : ptr_(array.data_ptr()), size_(N) {}

    T& operator[](size_t index) {
        #ifndef SCF_DONT_THROW
        if (index >= size_)
            throw scf::out_of_range("[SCF] span: out of range");
        #endif

        return ptr_[index];
    }

    const T& operator[](size_t index) const {
        #ifndef SCF_DONT_THROW
        if (index >= size_)
            throw scf::out_of_range("[SCF] span: out of range");
        #endif

        return ptr_[index];
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    T* begin() {
        return ptr_;
    }

    T* end() {
        return ptr_ + size_;
    }

    const T* begin() const {
        return ptr_;
    }

    const T* end() const {
        return ptr_ + size_;
    }

    T& front() {
        return ptr_[0];
    }

    const T& front() const {
        return ptr_[0];
    }

    T& back() {
        return ptr_[size_ - 1];
    }

    const T& back() const {
        return ptr_[size_ - 1];
    }

    T* data() {
        return ptr_;
    }

    const T* data() const {
        return ptr_;
    }
};

}