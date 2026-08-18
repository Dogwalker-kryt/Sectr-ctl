#pragma once

// #include <new>

#include "scf_exception.hpp"
#include "scf_type_traits.hpp"
#include "scf_object.hpp"

namespace scf {

template<typename T, typename E>
class result {
    static_assert(!scf::type_traits::is_reference_v<T>, "[SCF_result] T must not be a reference");
    static_assert(!scf::type_traits::is_reference_v<E>, "[SCF_result] E must not be a reference");

    alignas(T) unsigned char value_storage_[sizeof(T)];
    alignas(E) unsigned char error_storage_[sizeof(E)];
    bool has_value_ = false;

    T* value_ptr() noexcept {
        return reinterpret_cast<T*>(value_storage_);
    }

    const T* value_ptr() const noexcept {
        return reinterpret_cast<const T*>(value_storage_);
    }

    E* error_ptr() noexcept {
        return reinterpret_cast<E*>(error_storage_);
    }

    const E* error_ptr() const noexcept {
        return reinterpret_cast<const E*>(error_storage_);
    }

    void construct_value(const T& value) {
        ::new (static_cast<void*>(value_ptr())) T(value);
        has_value_ = true;
    }

    void construct_value(T&& value) {
        ::new (static_cast<void*>(value_ptr())) T(static_cast<T&&>(value));
        has_value_ = true;
    }

    void construct_error(const E& error) {
        ::new (static_cast<void*>(error_ptr())) E(error);
        has_value_ = false;
    }

    void construct_error(E&& error) {
        ::new (static_cast<void*>(error_ptr())) E(static_cast<E&&>(error));
        has_value_ = false;
    }

    void destroy() noexcept {
        if (has_value_) {
            value_ptr()->~T();
        } else {
            error_ptr()->~E();
        }
    }

public:
    result() noexcept : has_value_(false) {
        ::new (static_cast<void*>(error_ptr())) E();
    }

    result(const T& value) : has_value_(false) {
        construct_value(value);
    }

    result(T&& value) : has_value_(false) {
        construct_value(static_cast<T&&>(value));
    }

    result(const E& error) : has_value_(false) {
        construct_error(error);
    }

    result(E&& error) : has_value_(false) {
        construct_error(static_cast<E&&>(error));
    }

    result(const result& other) : has_value_(false) {
        if (other.has_value_) {
            construct_value(other.value());
        } else {
            construct_error(other.error());
        }
    }

    result(result&& other) : has_value_(false) {
        if (other.has_value_) {
            construct_value(static_cast<T&&>(other.value()));
        } else {
            construct_error(static_cast<E&&>(other.error()));
        }
        other.reset();
    }

    ~result() noexcept {
        if (has_value_) {
            value_ptr()->~T();
        } else {
            error_ptr()->~E();
        }
    }

    result& operator=(const result& other) {
        if (this == &other) {
            return *this;
        }

        if (has_value_ == other.has_value_) {
            if (has_value_) {
                *value_ptr() = other.value();
            } else {
                *error_ptr() = other.error();
            }
            return *this;
        }

        destroy();

        if (other.has_value_) {
            construct_value(other.value());
        } else {
            construct_error(other.error());
        }

        return *this;
    }

    result& operator=(result&& other) {
        if (this == &other) {
            return *this;
        }

        if (has_value_ == other.has_value_) {
            if (has_value_) {
                *value_ptr() = static_cast<T&&>(other.value());
            } else {
                *error_ptr() = static_cast<E&&>(other.error());
            }
            other.reset();
            return *this;
        }

        destroy();

        if (other.has_value_) {
            construct_value(static_cast<T&&>(other.value()));
        } else {
            construct_error(static_cast<E&&>(other.error()));
        }

        other.reset();
        return *this;
    }

    static result ok(const T& value) {
        return result(value);
    }

    static result ok(T&& value) {
        return result(static_cast<T&&>(value));
    }

    static result err(const E& error) {
        return result(error);
    }

    static result err(E&& error) {
        return result(static_cast<E&&>(error));
    }

    bool has_value() const noexcept {
        return has_value_;
    }

    bool has_error() const noexcept {
        return !has_value_;
    }

    explicit operator bool() const noexcept {
        return has_value_;
    }

    T& value() noexcept {
        return *value_ptr();
    }

    const T& value() const noexcept {
        return *value_ptr();
    }

    E& error() noexcept {
        return *error_ptr();
    }

    const E& error() const noexcept {
        return *error_ptr();
    }

    T value_or(const T& fallback) const noexcept {
        return has_value_ ? value() : fallback;
    }

    E error_or(const E& fallback) const noexcept {
        return has_value_ ? fallback : error();
    }

    T& unwrap() {
        if (!has_value_) {
            throw scf::runtime_error("[SCF_runtime_error] result::unwrap() called on error");
        }
        return value();
    }

    const T& unwrap() const {
        if (!has_value_) {
            throw scf::runtime_error("[SCF_runtime_error] result::unwrap() called on error");
        }
        return value();
    }

    E& unwrap_error() {
        if (has_value_) {
            throw scf::runtime_error("[SCF_runtime_error] result::unwrap_error() called on value");
        }
        return error();
    }

    const E& unwrap_error() const {
        if (has_value_) {
            throw scf::runtime_error("[SCF_runtime_error] result::unwrap_error() called on value");
        }
        return error();
    }

    template<typename F>
    auto map(F&& func) -> result<decltype(func(value())), E> {
        if (!has_value_) {
            return result<decltype(func(value())), E>::err(error());
        }
        return result<decltype(func(value())), E>::ok(func(value()));
    }

    template<typename F>
    auto map_error(F&& func) -> result<T, decltype(func(error()))> {
        if (has_value_) {
            return result<T, decltype(func(error()))>::ok(value());
        }
        return result<T, decltype(func(error()))>::err(func(error()));
    }

    void reset() noexcept {
        destroy();
        has_value_ = false;
    }
};

} // namespace scf
