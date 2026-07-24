/* 
 * SCF optional - Stack C++ Framework optional type
 * Copyright (C) 2026 Dogwalker-kryt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include "scf_exception.hpp"
#include "scf_str.hpp"
#include "scf_string_view.hpp"
#include "scf_cutils.hpp"
#include "scf_type_traits.hpp"

namespace scf {

// nullopt tag type
struct nullopt_t {
    explicit constexpr nullopt_t(int) {}
};
inline constexpr nullopt_t nullopt{0};

// in_place tag type
struct in_place_t {
    explicit constexpr in_place_t(int) {}
};
inline constexpr in_place_t in_place{0};

template<typename T>
class optional {
    static_assert(!scf::type_traits::is_reference_v<T>, "optional<T&> not supported in this implementation");

    alignas(T) unsigned char storage_[sizeof(T)];
    bool has_value_ = false;

    T* ptr() noexcept {
        return reinterpret_cast<T*>(storage_);
    }

    const T* ptr() const noexcept {
        return reinterpret_cast<const T*>(storage_);
    }

    void destroy() noexcept {
        if (has_value_) {
            ptr()->~T();
            has_value_ = false;
        }
    }

public:
    // constructors
    constexpr optional() noexcept = default;

    constexpr optional(nullopt_t) noexcept : has_value_(false) {}

    // optional(const T& value) {
    //     ::new (static_cast<void*>(storage_)) T(value);
    //     has_value_ = true;
    // }

    // str

    optional(const scf::str8& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str16& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str32& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str64& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str128& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str256& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str512& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str1024& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    #if INTPTR_MAX == INT64_MAX
    optional(const scf::str2048& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(const scf::str4096& value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }
    #endif


    // end str

    optional(T&& value) {
        ::new (static_cast<void*>(storage_)) T(move(value));
        has_value_ = true;
    }

    // C-style string constructors
    optional(const char* value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    template<size_t N>
    optional(const char (&value)[N]) {
        ::new (static_cast<void*>(storage_)) T(value, N - 1);
        has_value_ = true;
    }

    optional(char value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    optional(str_view value) {
        ::new (static_cast<void*>(storage_)) T(value);
        has_value_ = true;
    }

    template<typename... Args>
    explicit optional(in_place_t, Args&&... args) {
        ::new (static_cast<void*>(storage_)) T(scf::type_traits::forward<Args>(args)...);
        has_value_ = true;
    }

    optional(const optional& other) {
        if (other.has_value_) {
            ::new (static_cast<void*>(storage_)) T(*other.ptr());
            has_value_ = true;
        }
    }

    optional(optional&& other) noexcept(scf::type_traits::is_nothrow_move_assignable_v<T>) {
        if (other.has_value_) {
            ::new (static_cast<void*>(storage_)) T(move(*other.ptr()));
            has_value_ = true;
            other.destroy();
        }
    }

    // destructor
    ~optional() {
        destroy();
    }

    // assignment
    optional& operator=(nullopt_t) noexcept {
        destroy();
        return *this;
    }

    optional& operator=(const optional& other) {
        if (this == &other) return *this;

        if (has_value_ && other.has_value_) {
            *ptr() = *other.ptr();
        } else if (has_value_ && !other.has_value_) {
            destroy();
        } else if (!has_value_ && other.has_value_) {
            ::new (static_cast<void*>(storage_)) T(*other.ptr());
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(optional&& other) noexcept(
        scf::type_traits::is_nothrow_move_assignable_v<T> && scf::type_traits::is_nothrow_move_assignable_v<T>
    ) {
        if (this == &other) return *this;

        if (has_value_ && other.has_value_) {
            *ptr() = move(*other.ptr());
            other.destroy();
        } else if (has_value_ && !other.has_value_) {
            destroy();
        } else if (!has_value_ && other.has_value_) {
            ::new (static_cast<void*>(storage_)) T(move(*other.ptr()));
            has_value_ = true;
            other.destroy();
        }
        return *this;
    }

    // optional& operator=(const T& value) {
    //     if (has_value_) {
    //         *ptr() = value;
    //     } else {
    //         ::new (static_cast<void*>(storage_)) T(value);
    //         has_value_ = true;
    //     }
    //     return *this;
    // }

    // str
    optional& operator=(const scf::str8& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str16& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str32& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str64& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str128& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str256& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str512& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str1024& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    #if INTPTR_MAX == INT64_MAX
    optional& operator=(const scf::str2048& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(const scf::str4096& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }
    #endif

    // end str

    optional& operator=(T&& value) {
        if (has_value_) {
            *ptr() = move(value);
        } else {
            ::new (static_cast<void*>(storage_)) T(move(value));
            has_value_ = true;
        }
        return *this;
    }

    // C-style string assignment
    optional& operator=(const char* value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    template<size_t N>
    optional& operator=(const char (&value)[N]) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value, N - 1);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(char value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional& operator=(str_view value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            ::new (static_cast<void*>(storage_)) T(value);
            has_value_ = true;
        }
        return *this;
    }

    // observers
    constexpr bool has_value() const noexcept { return has_value_; }
    constexpr explicit operator bool() const noexcept { return has_value_; }

    T& value() & {
        if (!has_value_) throw scf::runtime_error("scf::optional: no value");
        return *ptr();
    }

    const T& value() const & {
        if (!has_value_) throw scf::runtime_error("scf::optional: no value");
        return *ptr();
    }

    T&& value() && {
        if (!has_value_) throw scf::runtime_error("scf::optional: no value");
        return move(*ptr());
    }

    const T&& value() const && {
        if (!has_value_) throw scf::runtime_error("scf::optional: no value");
        return move(*ptr());
    }

    T& operator*() & noexcept { return *ptr(); }
    const T& operator*() const & noexcept { return *ptr(); }
    T&& operator*() && noexcept { return move(*ptr()); }
    const T&& operator*() const && noexcept { return move(*ptr()); }

    T* operator->() noexcept { return ptr(); }
    const T* operator->() const noexcept { return ptr(); }

    template<typename U>
    T value_or(U&& default_value) const {
        return has_value_ ? *ptr() : static_cast<T>(scf::type_traits::forward<U>(default_value));
    }

    // modifiers
    void reset() noexcept {
        destroy();
    }

    template<typename... Args>
    T& emplace(Args&&... args) {
        destroy();
        ::new (static_cast<void*>(storage_)) T(scf::type_traits::forward<Args>(args)...);
        has_value_ = true;
        return *ptr();
    }

    void op_swap(optional& other) noexcept(
        scf::type_traits::is_nothrow_move_assignable_v<T> && scf::type_traits::is_nothrow_swappable_v<T>
    ) {
        

        if (has_value_ && other.has_value_) {
            swap(*ptr(), *other.ptr());
        } else if (has_value_ && !other.has_value_) {
            other.emplace(move(*ptr()));
            destroy();
        } else if (!has_value_ && other.has_value_) {
            emplace(move(*other.ptr()));
            other.destroy();
        }
    }
};

// comparisons
template<typename T>
bool operator==(const optional<T>& a, const optional<T>& b) {
    if (a.has_value() != b.has_value()) return false;
    if (!a.has_value()) return true;
    return *a == *b;
}

template<typename T>
bool operator!=(const optional<T>& a, const optional<T>& b) {
    return !(a == b);
}

template<typename T>
bool operator==(const optional<T>& a, nullopt_t) noexcept {
    return !a.has_value();
}

template<typename T>
bool operator==(nullopt_t, const optional<T>& a) noexcept {
    return !a.has_value();
}

template<typename T>
bool operator!=(const optional<T>& a, nullopt_t) noexcept {
    return a.has_value();
}

template<typename T>
bool operator!=(nullopt_t, const optional<T>& a) noexcept {
    return a.has_value();
}

template<typename T>
bool operator==(const optional<T>& a, const T& v) {
    return a.has_value() && *a == v;
}

template<typename T>
bool operator==(const T& v, const optional<T>& a) {
    return a.has_value() && v == *a;
}

template<typename T>
bool operator!=(const optional<T>& a, const T& v) {
    return !(a == v);
}

template<typename T>
bool operator!=(const T& v, const optional<T>& a) {
    return !(v == a);
}

// c string comparisons
// optional<str32> == const char*
template<typename T>
bool operator==(const optional<T>& a, const char* v) {
    return a.has_value() && *a == v;
}

// const char* == optional<str32>
template<typename T>
bool operator==(const char* v, const optional<T>& a) {
    return a.has_value() && *a == v;
}

// optional<str32> == const char[N]
template<typename T, size_t N>
bool operator==(const optional<T>& a, const char (&v)[N]) {
    return a.has_value() && *a == v;
}

// const char[N] == optional<str32>
template<typename T, size_t N>
bool operator==(const char (&v)[N], const optional<T>& a) {
    return a.has_value() && *a == v;
}

// helper
template<typename T, typename... Args>
optional<T> make_optional(Args&&... args) {
    return optional<T>(in_place, scf::type_traits::forward<Args>(args)...);
}

template<typename T>
struct is_optional : scf::type_traits::false_type {};

template<typename T>
struct is_optional<optional<T>> : scf::type_traits::true_type {};

} // namespace scf