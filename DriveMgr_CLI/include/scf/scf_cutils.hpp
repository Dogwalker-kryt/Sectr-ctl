// scf_cutils.hpp
#pragma once

#include <stdio.h>
#include "scf_cutils.hpp"
#include "scf_type_traits.hpp"
#include <charconv>
#include <unistd.h>

namespace scf {

// --- String Length ---
inline size_t strlen(const char* str) {
    if (!str) return 0;
    size_t len = 0;
    while (str[len] != '\0') ++len;
    return len;
}

// --- String copy n ---
inline char* strncpy(char* dest, const char* src, size_t n) {
    for (size_t i = 0; i < n && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }
    // Pad the rest with null terminators (optional)
    for (size_t i = strlen(src); i < n; ++i) {
        dest[i] = '\0';
    }
    return dest;
}

// --- Memory Copy ---
inline void* memcpy(void* dest, const void* src, size_t n) {
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

// --- Memory Compare ---
inline int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = static_cast<const unsigned char*>(s1);
    const unsigned char* p2 = static_cast<const unsigned char*>(s2);
    for (size_t i = 0; i < n; ++i) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    return 0;
}

inline constexpr int constexpr_memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = static_cast<const unsigned char*>(s1);
    const unsigned char* p2 = static_cast<const unsigned char*>(s2);
    for (size_t i = 0; i < n; ++i) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    return 0;
}

// --- Memory Set ---
inline void* memset(void* s, int c, size_t n) {
    unsigned char* p = static_cast<unsigned char*>(s);
    for (size_t i = 0; i < n; ++i) {
        p[i] = static_cast<unsigned char>(c);
    }
    return s;
}

// --- Fill N ---
template<typename T>
inline void fill_n(T* dest, size_t count, const T& value) {
    for (size_t i = 0; i < count; ++i) {
        dest[i] = value;
    }
}

// --- Copy N ---
template<typename T>
inline void copy_n(const T* src, size_t count, T* dest) {
    for (size_t i = 0; i < count; ++i) {
        dest[i] = src[i];
    }
}

template<typename T>
inline void copy_backwards(const T* src, size_t count, T* dest) {
    for (size_t i = 0; i < count; ++i) {
        dest[count - 1 - i] = src[count - 1 - i];
    }
}

template<typename T>
inline void copy(const T* src, const T* src_end, T* dest) {
    while (src != src_end) {
        *dest++ = *src++;
    }
}

// --- Min/Max ---
template<typename T>
inline const T& min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
inline constexpr const T& constexpr_min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
inline const T& max(const T& a, const T& b) {
    return a > b ? a : b;
}

template<typename T>
inline constexpr T& constexpr_max(const T& a, const T& b) {
    return a > b ? a : b;
}

// --- Move (for move semantics) ---
template<typename T>
inline T&& move(T& x) noexcept {
    return static_cast<T&&>(x);
}

// --- Swap ---
template<typename T>
inline void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

// --- remove_reference ---
template<typename T>
struct remove_reference {
    using type = T;
};

template<typename T>
struct remove_reference<T&> {
    using type = T;
};

template<typename T>
struct remove_reference<T&&> {
    using type = T;
};

template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

// --- move ---
template<typename T>
constexpr remove_reference_t<T>&& move(T&& x) noexcept {
    return static_cast<remove_reference_t<T>&&>(x);
}

// --- forward ---
template<typename T>
constexpr T&& scf::type_traits::forward(remove_reference_t<T>& x) noexcept {
    return static_cast<T&&>(x);
}

template<typename T>
constexpr T&& scf::type_traits::forward(remove_reference_t<T>&& x) noexcept {
    return static_cast<T&&>(x);
}


} // namespace scf