
#pragma once
#include "scf_cutils.hpp"
#include "scf_exception.hpp"

template<size_t N>
class fxdstr;

namespace scf {

class str_view {
    const char* data_ = nullptr;
    size_t size_ = 0;

public:
    // Constructors
    constexpr str_view() noexcept = default;
    constexpr str_view(const char* str)
        : data_(str), size_(str ? strlen(str) : 0) {}
    constexpr str_view(const char* str, size_t size)
        : data_(str), size_(size) {}

    template<size_t N>
    constexpr str_view(const char (&str)[N])
        : data_(str), size_(N - 1) {} // Exclude null terminator

    constexpr str_view(const char c)
        : data_(&c), size_(1) {}

    // Accessors
    constexpr const char* data() const noexcept { return data_; }
    constexpr size_t size() const noexcept { return size_; }
    constexpr size_t length() const noexcept { return size_; }
    constexpr bool empty() const noexcept { return size_ == 0; }

    // Element access
    constexpr const char& operator[](size_t pos) const { return data_[pos]; }
    constexpr const char& at(size_t pos) const {
        if (pos >= size_) throw length_error("[SCF_length_error] str_view::at: out of range");
        return data_[pos];
    }

    // Comparison
    constexpr bool operator==(str_view other) const noexcept {
        if (size_ != other.size_) return false;
        return memcmp(data_, other.data_, size_) == 0;
    }
    constexpr bool operator!=(str_view other) const noexcept {
        return !(*this == other);
    }
    constexpr bool operator<(str_view other) const noexcept {
        int cmp = constexpr_memcmp(data_, other.data_, constexpr_min(size_, other.size_));
        if (cmp != 0) return cmp < 0;
        return size_ < other.size_;
    }
    // Add other comparison operators as needed

    // Substring
    constexpr str_view substr(size_t pos, size_t count = npos) const {
        if (pos > size_) throw length_error("[SCF_length_error] str_view::substr: pos out of range");
        count = constexpr_min(count, size_ - pos);
        return str_view(data_ + pos, count);
    }

    // Find
    constexpr size_t find(char c, size_t pos = 0) const {
        for (size_t i = pos; i < size_; ++i) {
            if (data_[i] == c) return i;
        }
        return npos;
    }
    constexpr size_t find(str_view str, size_t pos = 0) const {
        if (str.empty() || str.size_ > size_) return npos;
        for (size_t i = pos; i <= size_ - str.size_; ++i) {
            if (memcmp(data_ + i, str.data_, str.size_) == 0) return i;
        }
        return npos;
    }

    // Conversion
    template<size_t N>
    explicit operator fxdstr<N>() const {
        return fxdstr<N>(data_, size_);
    }

    static constexpr size_t npos = static_cast<size_t>(-1);
};

} // namespace scf