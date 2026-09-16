// scf_cutils.hpp
#pragma once

#include "scf_type_traits.hpp"
#include <limits.h>
#include <stdalign.h>

namespace scf {
constexpr size_t npos = static_cast<size_t>(-1);
// --- String Length ---

/**
 * @brief strlen
 * @warning str has to be garantee no NULL/nullptr
 * @returns npos [ (size_t)18446744073709551615UL ] if fails
 * @return self explanatory
 */
inline size_t strlen(const char* str) noexcept {
    const char *s = str;
    while (*s) s++;
    return s - str;
}

/**
 * @brief little bit better strnlen
 * @returns npos [ (size_t)18446744073709551615UL ] if fails
 * @return self explanatory
 */
// inline size_t strnlen(const char *str, size_t max_n) noexcept {
//     if (!str) return npos;
//     size_t len = 0;
//     while (len < max_n && str[len] != '\0') ++len;
//     return len;
// }


// // --- String copy n ---

/**
 * @brief Copy up to n characters from src to dest, no null-termination.
 * @param dest Must have at least n bytes.
 * @param src Must be valid.
 * @param n Number of characters to copy.
 * @return dest.
 * @warning Does NOT null-terminate. UB if dest or src is null.
 */
// inline char* strncpy(char* dest, const char* src, size_t n) {
//     size_t i{};

//     while (i < n && src[i] != '\0') {
//         dest[i] = src[i];
//         ++i;
//     }

//     return dest;
// }

/**
 * @brief Copy up to n-1 characters from src to dest, always null-terminates.
 * @param dest Must have at least n bytes.
 * @param src Must be valid.
 * @param n Maximum number of characters to copy (including null terminator).
 * @return dest.
 * @warning UB if dest or src is null.
 */
// inline char* strncpy_nt(char* dest, const char* src, size_t n) {
//     size_t i{};

//     while (i < n - 1 && src[i] != '\0') {
//         dest[i] = src[i];
//         ++i;
//     }
    
//     dest[i] = '\0';

//     return dest;
// }

/**
 * @brief Copy src to dest, ensuring null-termination, and return src length.
 * @param dest Destination buffer.
 * @param src Source string.
 * @param dest_len Size of the destination buffer.
 * @return Length of src.
 * @warning UB if dest or src is null.
 */
// inline size_t strlcpy(char* dest, const char* src, size_t dest_len) {
//     if (dest_len == 0) return strlen(src);
//     size_t src_len = strlen(src);
//     size_t len = (src_len < dest_len - 1) ? src_len : dest_len - 1;
//     for (size_t i = 0; i < len; ++i) {
//         dest[i] = src[i];
//     }
//     dest[len] = '\0';
//     return src_len;
// }
// inline char* strlcpy(char* dest, const char* src, const size_t dest_len) {
//     const size_t src_len = strlen(src);

//     if (dest_len < src_len + 1) {
//         return nullptr;
//     }

//     size_t i{};

//     while (i < src_len) {
//         dest[i] = src[i];
//         ++i;
//     }

//     dest[src_len] = '\0';

//     return dest;
// }

// --- Memory Copy ---
inline void* memcpy(void* dest, const void* src, size_t n) noexcept {
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

// --- Memory Compare ---
inline int memcmp(const void* s1, const void* s2, size_t n) noexcept {
    const unsigned char* p1 = static_cast<const unsigned char*>(s1);
    const unsigned char* p2 = static_cast<const unsigned char*>(s2);
    for (size_t i = 0; i < n; ++i) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    return 0;
}

inline constexpr int constexpr_memcmp(const void* s1, const void* s2, size_t n) noexcept  {
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

// --- Memory Move ---
inline void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);

    if (d < s) {
        for (size_t i = 0; i < n; ++i) {
            d[i] = s[i];
        }
    } else if (d > s) {
        for (size_t i = n; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }

    return dest;
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

// --- conversions ---

template<typename T>
inline constexpr unsigned long long max_integral_value() {
    if constexpr (scf::type_traits::is_same_v<T, unsigned long long>) return ULLONG_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, unsigned long>) return ULONG_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, unsigned int>) return UINT_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, unsigned short>) return USHRT_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, unsigned char>) return UCHAR_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, char>) {
        if constexpr (scf::type_traits::is_signed_v<char>) {
            return static_cast<unsigned long long>(CHAR_MAX);
        } else {
            return static_cast<unsigned long long>(UCHAR_MAX);
        }
    }
    return 0ULL;
}

template<typename T>
inline constexpr long long min_integral_value() {
    if constexpr (scf::type_traits::is_same_v<T, long long>) return LLONG_MIN;
    else if constexpr (scf::type_traits::is_same_v<T, long>) return LONG_MIN;
    else if constexpr (scf::type_traits::is_same_v<T, int>) return INT_MIN;
    else if constexpr (scf::type_traits::is_same_v<T, short>) return SHRT_MIN;
    else if constexpr (scf::type_traits::is_same_v<T, signed char>) return SCHAR_MIN;
    else if constexpr (scf::type_traits::is_same_v<T, char>) return CHAR_MIN;
    return 0LL;
}

template<typename T>
inline constexpr long long max_signed_integral_value() {
    if constexpr (scf::type_traits::is_same_v<T, long long>) return LLONG_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, long>) return LONG_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, int>) return INT_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, short>) return SHRT_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, signed char>) return SCHAR_MAX;
    else if constexpr (scf::type_traits::is_same_v<T, char>) return CHAR_MAX;
    return 0LL;
}

template<typename T>
inline int integer_to_cstr(T value, char* out, int max) {
    static_assert(scf::type_traits::is_integral_v<T>, "integer_to_cstr requires an integral type");
    if (max <= 0) return 0;

    bool negative = false;
    unsigned long long u = 0ULL;

    if constexpr (scf::type_traits::is_signed_v<T>) {
        if (value < 0) {
            negative = true;
            u = static_cast<unsigned long long>(-static_cast<long long>(value));
        } else {
            u = static_cast<unsigned long long>(value);
        }
    } else {
        u = static_cast<unsigned long long>(value);
    }

    char temp[64];
    int len = 0;

    if (u == 0ULL) {
        temp[len++] = '0';
    } else {
        while (u != 0ULL) {
            temp[len++] = static_cast<char>('0' + (u % 10ULL));
            u /= 10ULL;
        }
    }

    if (negative) {
        temp[len++] = '-';
    }

    if (len + 1 > max) {
        out[0] = '\0';
        return 0;
    }

    for (int i = 0; i < len; ++i) {
        out[i] = temp[len - 1 - i];
    }
    out[len] = '\0';
    return len;
}

template<typename T>
inline bool cstr_to_integral(const char* first, const char* last, T& out) {
    static_assert(scf::type_traits::is_integral_v<T>, "cstr_to_integral requires an integral type");
    if (!first || !last || first >= last) return false;

    const char* p = first;
    bool negative = false;

    if (*p == '+' || *p == '-') {
        negative = (*p == '-');
        ++p;
    }

    if (p == last) return false;

    unsigned long long accum = 0ULL;
    bool any_digit = false;

    while (p != last && *p >= '0' && *p <= '9') {

        any_digit = true;
        unsigned long long digit = static_cast<unsigned long long>(*p - '0');

        if (accum > ULLONG_MAX / 10ULL || (accum == ULLONG_MAX / 10ULL && digit > ULLONG_MAX % 10ULL)) {
            return false;
        }

        accum = accum * 10ULL + digit;
        ++p;
    }

    if (!any_digit || p != last) return false;

    if constexpr (scf::type_traits::is_unsigned_v<T>) {

        if (negative) return false;
        if (accum > max_integral_value<T>()) return false;
        out = static_cast<T>(accum);
        return true;

    } else {

        if (negative) {

            unsigned long long limit = static_cast<unsigned long long>(-(min_integral_value<T>() + 1LL)) + 1ULL;
            if (accum > limit) return false;

            if (accum == limit) {
                out = static_cast<T>(min_integral_value<T>());
            } else {
                out = static_cast<T>(-static_cast<long long>(accum));
            }

            return true;

        } else {

            if (accum > static_cast<unsigned long long>(max_signed_integral_value<T>())) return false;

            out = static_cast<T>(accum);

            return true;

        }
    }
}

} // namespace scf