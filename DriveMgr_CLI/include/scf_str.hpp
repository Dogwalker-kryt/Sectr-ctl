/* 
 * SCF STR - Stack C++ Framework Fixed String type
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

#include <array>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstdio> 
#include <cstdint>
#include <string>
#include <type_traits>
#include <string_view>
#include <iostream> 

/**
 * @class fxdstr
 * @brief A fixed-size, stack-allocated string class.
 * @tparam N Maximum length of the string (excluding null terminator).
 *
 * fxdstr is a lightweight, non-allocating string class for use in
 * performance-critical or embedded contexts. It does not depend on std::string.
 */
template<size_t N>
class fxdstr {
    std::array<char, N + 1> buffer_ = {};
    size_t len = 0;

public:
    using iterator = char*;
    using const_iterator = const char*;
    static constexpr size_t npos = static_cast<size_t>(-1);

    // --- Constructors ---
    fxdstr() = default;
    fxdstr(const fxdstr&) = default;
    fxdstr& operator=(const fxdstr&) = default;

    fxdstr(const char* str) {
        if (str) {
            size_t n = std::strlen(str);
            if (n > N) throw std::length_error("String too long");
            std::strncpy(buffer_.data(), str, N);
            len = n;
        }
    }

    fxdstr(size_t count, char c) {
        if (count > N) throw std::length_error("String too long");
        std::fill_n(buffer_.data(), count, c);
        buffer_[count] = '\0';
        len = count;
    }

    template<size_t M>
    fxdstr(const fxdstr<M>& other) {
        if (other.size() > N) throw std::length_error("String too long");
        std::copy(other.begin(), other.end(), buffer_.data());
        buffer_[other.size()] = '\0';
        len = other.size();
    }

    // Constructor from std::string
    fxdstr(const std::string& str) {
        if (str.size() > N) throw std::length_error("String too long");
        std::copy(str.begin(), str.end(), buffer_.data());
        buffer_[str.size()] = '\0';
        len = str.size();
    }

    // Constructor from std::string_view
    fxdstr(std::string_view sv) {
        if (sv.size() > N) throw std::length_error("String too long");
        std::copy(sv.begin(), sv.end(), buffer_.data());
        buffer_[sv.size()] = '\0';
        len = sv.size();
    }

    // --- Assignment ---
    fxdstr& operator=(const char* str) {
        if (str) {
            size_t n = std::strlen(str);
            if (n > N) throw std::length_error("String too long");
            std::strcpy(buffer_.data(), str);
            len = n;
        } else {
            clear();
        }
        return *this;
    }

    fxdstr& operator=(char c) {
        buffer_[0] = c;
        buffer_[1] = '\0';
        len = 1;
        return *this;
    }

    fxdstr& operator=(const std::string& str) {
        if (str.size() > N) throw std::length_error("String too long");
        std::copy(str.begin(), str.end(), buffer_.data());
        buffer_[str.size()] = '\0';
        len = str.size();
        return *this;
    }

    fxdstr& operator=(std::string_view sv) {
        if (sv.size() > N) throw std::length_error("String too long");
        std::copy(sv.begin(), sv.end(), buffer_.data());
        buffer_[sv.size()] = '\0';
        len = sv.size();
        return *this;
    }

    template<size_t M>
    fxdstr& operator=(const fxdstr<M>& other) {
        if (other.size() > N) throw std::length_error("String too long");
        std::copy(other.begin(), other.end(), buffer_.data());
        buffer_[other.size()] = '\0';
        len = other.size();
        return *this;
    }

    // --- Element access ---
    char& operator[](size_t pos) { return buffer_[pos]; }
    const char& operator[](size_t pos) const { return buffer_[pos]; }
    char& at(size_t pos) {
        if (pos >= len) throw std::out_of_range("fxdstr::at");
        return buffer_[pos];
    }
    const char& at(size_t pos) const {
        if (pos >= len) throw std::out_of_range("fxdstr::at");
        return buffer_[pos];
    }
    char& front() { return buffer_[0]; }
    const char& front() const { return buffer_[0]; }
    char& back() { return buffer_[len - 1]; }
    const char& back() const { return buffer_[len - 1]; }
    const char* c_str() const { return buffer_.data(); }
    const char* data() const { return buffer_.data(); }
    char* data() { return buffer_.data(); }

    // --- Iterators ---
    iterator begin() { return buffer_.data(); }
    const_iterator begin() const { return buffer_.data(); }
    iterator end() { return buffer_.data() + len; }
    const_iterator end() const { return buffer_.data() + len; }

    // --- Capacity ---
    size_t size() const { return len; }
    size_t length() const { return len; }
    size_t max_size() const { return N; }
    size_t capacity() const { return N; }
    bool empty() const { return len == 0; }

    // Set length (caller must ensure buffer is null-terminated and valid)
    void set_length(size_t new_len) {
        if (new_len > N) throw std::length_error("fxdstr::set_length - length exceeds capacity");
        len = new_len;
        buffer_[len] = '\0';
    }

    // Direct access to length for low-level operations (use with care!)
    size_t& length_ref() { return len; }

    // --- Modifiers ---
    void clear() { buffer_[0] = '\0'; len = 0; }

    fxdstr& append(const char* str, size_t count) {
        if (len + count > N) throw std::length_error("String too long");
        std::copy(str, str + count, buffer_.data() + len);
        len += count;
        buffer_[len] = '\0';
        return *this;
    }
    fxdstr& append(const char* str) {
        if (str) {
            size_t n = std::strlen(str);
            if (len + n > N) throw std::length_error("String too long");
            std::copy(str, str + n, buffer_.data() + len);
            len += n;
            buffer_[len] = '\0';
        }
        return *this;
    }
    fxdstr& append(size_t count, char c) {
        if (len + count > N) throw std::length_error("String too long");
        std::fill_n(buffer_.data() + len, count, c);
        len += count;
        buffer_[len] = '\0';
        return *this;
    }
    fxdstr& append(const fxdstr& other) {
        return append(other.c_str(), other.size());
    }

    fxdstr& operator+=(const fxdstr& other) { return append(other); }
    fxdstr& operator+=(const char* str) { return append(str); }
    fxdstr& operator+=(char c) { push_back(c); return *this; }

    void push_back(char c) {
        if (len + 1 > N) throw std::length_error("String too long");
        buffer_[len++] = c;
        buffer_[len] = '\0';
    }
    void pop_back() {
        if (len == 0) throw std::out_of_range("fxdstr::pop_back");
        --len;
        buffer_[len] = '\0';
    }

    // --- Modifiers (erase, replace) ---
    fxdstr& erase(size_t pos = 0, size_t count = npos) {
        if (pos > len) throw std::out_of_range("fxdstr::erase");
        if (count == npos) {
            len = pos;
            buffer_[len] = '\0';
            return *this;
        }
        size_t erase_count = std::min(count, len - pos);
        std::copy(buffer_.data() + pos + erase_count, buffer_.data() + len,
                  buffer_.data() + pos);
        len -= erase_count;
        buffer_[len] = '\0';
        return *this;
    }

    fxdstr& replace(size_t pos, size_t count, const char* str) {
        if (pos > len) throw std::out_of_range("fxdstr::replace");
        size_t replace_count = std::min(count, len - pos);
        size_t str_len = std::strlen(str);
        
        // Calculate resulting length
        size_t new_len = len - replace_count + str_len;
        if (new_len > N) throw std::length_error("fxdstr::replace - result too long");
        
        // Move tail to make room (or shrink)
        if (str_len > replace_count) {
            std::copy_backward(buffer_.data() + pos + replace_count, buffer_.data() + len,
                             buffer_.data() + pos + str_len + (len - pos - replace_count));
        } else if (str_len < replace_count) {
            std::copy(buffer_.data() + pos + replace_count, buffer_.data() + len,
                     buffer_.data() + pos + str_len);
        }
        
        // Copy in new string
        std::copy(str, str + str_len, buffer_.data() + pos);
        len = new_len;
        buffer_[len] = '\0';
        return *this;
    }

    fxdstr& replace(size_t pos, size_t count, const fxdstr& str) {
        return replace(pos, count, str.c_str());
    }

    // (modifier) actual size

    // size_t get_actual_size() const {
    //     static constexpr size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    //     size_t actual = len + 1;

    //     for (size_t s : sizes)
    //         if (actual <= s)
    //             return s;

    //     throw std::length_error("fxdstr::get_actual_size - actual size of string somehow is exeding 4096");
    //     return SIZE_MAX;
    // }

    // // resize to actual size (use with caution, as it may cause stack overflow if actual size is large)
    // void resize_to_actual_size() {
    //     size_t actual_size = get_actual_size();
    //     if (actual_size > N) throw std::length_error("fxdstr::resize_to_actual_size - actual size exceeds capacity");
    //     // create new str variable with the actual size and copy the data over with the same name as the original
    //     fxdstr<actual_size> resized(*this);
    //     *this = resized; // assign the resized string back to this
    // }

    // --- Comparison ---
    int compare(const fxdstr& other) const {
        size_t n = std::min(len, other.len);
        int cmp = std::memcmp(buffer_.data(), other.buffer_.data(), n);
        if (cmp != 0) return cmp;
        if (len < other.len) return -1;
        if (len > other.len) return 1;
        return 0;
    }
    int compare(const char* str) const {
        return compare(fxdstr<N>(str));
    }

    bool operator==(const fxdstr& other) const { return compare(other) == 0; }
    bool operator!=(const fxdstr& other) const { return compare(other) != 0; }
    bool operator<(const fxdstr& other) const { return compare(other) < 0; }
    bool operator>(const fxdstr& other) const { return compare(other) > 0; }
    bool operator<=(const fxdstr& other) const { return compare(other) <= 0; }
    bool operator>=(const fxdstr& other) const { return compare(other) >= 0; }

    bool operator==(const char* str) const { return compare(str) == 0; }
    bool operator!=(const char* str) const { return compare(str) != 0; }

    // --- String predicates (C++20 style) ---
    bool starts_with(const char* str) const {
        size_t n = std::strlen(str);
        if (n > len) return false;
        return std::memcmp(buffer_.data(), str, n) == 0;
    }
    bool starts_with(char c) const {
        return len > 0 && buffer_[0] == c;
    }
    bool starts_with(const fxdstr& str) const {
        return starts_with(str.c_str());
    }

    bool ends_with(const char* str) const {
        size_t n = std::strlen(str);
        if (n > len) return false;
        return std::memcmp(buffer_.data() + len - n, str, n) == 0;
    }
    bool ends_with(char c) const {
        return len > 0 && buffer_[len - 1] == c;
    }
    bool ends_with(const fxdstr& str) const {
        return ends_with(str.c_str());
    }

    // --- Utility ---
    void swap(fxdstr& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(len, other.len);
    }

    // --- Substring ---
    fxdstr substr(size_t pos = 0, size_t count = npos) const {
        if (pos > len) throw std::out_of_range("fxdstr::substr");
        count = std::min(count, len - pos);
        fxdstr result;
        std::copy(buffer_.data() + pos, buffer_.data() + pos + count, result.buffer_.data());
        result.buffer_[count] = '\0';
        result.len = count;
        return result;
    }

    // Templated substr for smart-sizing (returns appropriate fxdstr<M>)
    template<size_t M>
    fxdstr<M> substr_sized(size_t pos = 0, size_t count = npos) const {
        if (pos > len) throw std::out_of_range("fxdstr::substr_sized");
        count = std::min(count, len - pos);
        if (count > M) throw std::length_error("fxdstr::substr_sized - result too large for target size");
        fxdstr<M> result;
        std::copy(buffer_.data() + pos, buffer_.data() + pos + count, result.buffer_.data());
        result.buffer_[count] = '\0';
        result.len = count;
        return result;
    }

    // --- Search ---

    // find returns npos if not found, otherwise returns the index of the first occurrence
    size_t find(const char* str, size_t pos = 0) const {
        size_t n = std::strlen(str);
        if (pos > len || n == 0 || n > len) return npos;
        for (size_t i = pos; i <= len - n; ++i) {
            if (std::memcmp(buffer_.data() + i, str, n) == 0) return i;
        }
        return npos;
    }
    // find returns npos if not found, otherwise returns the index of the first occurrence
    size_t find(char c, size_t pos = 0) const {
        for (size_t i = pos; i < len; ++i) {
            if (buffer_[i] == c) return i;
        }
        return npos;
    }
    // find returns npos if not found, otherwise returns the index of the first occurrence
    size_t find(const std::string& str, size_t pos = 0) const {
        return find(str.c_str(), pos);
    }
    // find returns npos if not found, otherwise returns the index of the first occurrence
    size_t find(std::string_view sv, size_t pos = 0) const {
        if (sv.empty() || sv.size() > len) return npos;
        if (pos > len || pos + sv.size() > len) return npos;
        for (size_t i = pos; i <= len - sv.size(); ++i) {
            if (std::memcmp(buffer_.data() + i, sv.data(), sv.size()) == 0) return i;
        }
        return npos;
    }
    template<size_t M>
    size_t find(const fxdstr<M>& str, size_t offset = 0) const {
        return find(std::string_view(str), offset);
    }
    // same as find but just searches from right to left
    size_t rfind(const char* str, size_t pos = npos) const {
        size_t n = std::strlen(str);
        if (n == 0 || n > len) return npos;
        if (pos == npos) pos = len;
        if (pos > len) pos = len;
        for (size_t i = pos; i > 0; --i) {
            if (i - 1 >= len - n && i - 1 <= len) continue;
            if (std::memcmp(buffer_.data() + i - n, str, n) == 0) return i - n;
        }
        return npos;
    }
    // same as find but just searches from right to left
    size_t rfind(char c, size_t pos = npos) const {
        if (len == 0) return npos;
        if (pos == npos) pos = len - 1;
        if (pos >= len) pos = len - 1;
        for (size_t i = pos; i > 0; --i) {
            if (buffer_[i] == c) return i;
        }
        if (buffer_[0] == c) return 0;
        return npos;
    }
    size_t rfind(const std::string& str, size_t pos = npos) const {
        return rfind(str.c_str(), pos);
    }
    size_t rfind(std::string_view sv, size_t pos = npos) const {
        if (sv.empty() || sv.size() > len) return npos;
        if (pos == npos) pos = len;
        if (pos > len) pos = len;
        for (size_t i = pos; i > 0; --i) {
            if (i - 1 >= len - sv.size() && i - 1 <= len) continue;
            if (std::memcmp(buffer_.data() + i - sv.size(), sv.data(), sv.size()) == 0) return i - sv.size();
        }
        return npos;
    }
    template<size_t M>
    size_t rfind(const fxdstr<M>& str, size_t offset = npos) const {
        return rfind(std::string_view(str), offset);
    }

    // --- conversion by methods ---

    // pretty self explanatory
    template<size_t J>
    std::string to_std_str() {
        return std::string(buffer_.data(), len);
    }

    // --- C++17 string_view compatibility ---
    operator std::string_view() const { return std::string_view(buffer_.data(), len); }
    
    // Conversion to std::string
    operator std::string() const { return std::string(buffer_.data(), len); }
};



// --- Non-member functions ---

// fxdstr + fxdstr
template<size_t N, size_t M>
fxdstr<N + M> operator+(const fxdstr<N>& lhs, const fxdstr<M>& rhs) {
    fxdstr<N + M> result;
    result.append(lhs);
    result.append(rhs);
    return result;
}

// fxdstr + C-string literal ("text")
template<size_t N, size_t L>
fxdstr<N + (L - 1)> operator+(const fxdstr<N>& lhs, const char(&rhs)[L]) {
    fxdstr<N + (L - 1)> result;
    result.append(lhs);
    result.append(rhs);
    return result;
}

// C-string literal ("text") + fxdstr
template<size_t L, size_t N>
fxdstr<(L - 1) + N> operator+(const char(&lhs)[L], const fxdstr<N>& rhs) {
    fxdstr<(L - 1) + N> result;
    result.append(lhs);
    result.append(rhs);
    return result;
}

// fxdstr + char
template<size_t N>
fxdstr<N + 1> operator+(const fxdstr<N>& lhs, char rhs) {
    fxdstr<N + 1> result;
    result.append(lhs);
    result.push_back(rhs);
    return result;
}

// char + fxdstr
template<size_t N>
fxdstr<1 + N> operator+(char lhs, const fxdstr<N>& rhs) {
    fxdstr<1 + N> result;
    result.push_back(lhs);
    result.append(rhs);
    return result;
}

// --- Stream Operators ---
template<size_t N>
std::ostream& operator<<(std::ostream& os, const fxdstr<N>& str) {
    os.write(str.c_str(), str.size());
    return os;
}

template<size_t N>
std::istream& operator>>(std::istream& is, fxdstr<N>& str) {
    str.clear();
    char c;
    while (is.get(c) && !std::isspace(c)) {
        if (str.size() >= N) {
            is.setstate(std::ios::failbit);
            break;
        }
        str.push_back(c);
    }
    return is;
}

// --- Hash support ---
namespace std {
    template<size_t N>
    struct hash<fxdstr<N>> {
        size_t operator()(const fxdstr<N>& s) const {
            return std::hash<std::string_view>{}(std::string_view(s.c_str(), s.size()));
        }
    };
}

namespace scf {
    // --- Convenient aliases ---

    // pre fixed sizes

    using str8 = fxdstr<8>; // 8 characters, for very short strings or identifiers
    using str16 = fxdstr<16>; // 16 characters, for short strings, small identifiers, or short metadata fields
    using str32 = fxdstr<32>; // 32 characters, for medium-length strings, common metadata fields, or short combined metadata
    using str64 = fxdstr<64>; // 64 characters, for longer strings, larger buffers fields, or combined metadata
    using str128 = fxdstr<128>; // 128 characters, for long strings, large buffer fields, or combined/normal metadata. 
    using str256 = fxdstr<256>; // 256 characters, for very long strings, large buffer fields, or combined metadata. Use with caution due to potential stack overflow on 32-bit systems
    using str512 = fxdstr<512>; // 512 characters, for very long strings, large buffer fields, or combined metadata. Use with caution due to potential stack overflow on 32-bit systems
    #if INTPTR_MAX == INT32_MAX
        using str1024 = fxdstr<1024>; // (32-bit) 1KB string, should be enough for most metadata fields. Use of str1024 is not recommended on 32-bit systems  
    #else
        using str1024 = fxdstr<1024>; //(64-bit) 1KB string, should be enough for most metadata fields
    #endif
    
    // big sizes

    #if INTPTR_MAX == INT64_MAX
        using str2048 = fxdstr<2048>; // (64-bit) 2KB string, for larger metadata fields or combined metadata. Caution: heavy use of it may lead to stack overflow
        using str4096 = fxdstr<4096>; // (64-bit) 4KB string, for very large metadata, combined metadata, big buffers. Use with extreme caution due to high risk of stack overflow
    #endif

    // dynamic fixed sizes 

    // generic alias for fxdstr with size N, can be used for template parameters. 32-bit systems should generally not use sizes larger than 1024 to avoid stack overflow, while 64-bit systems can safely use sizes up to 2048 or 4096. Use with caution on 32-bit systems.
    template<size_t N>
    using fxdstr_t = fxdstr<N>; 

    // default string type, with size N. same as fxdstr_t<N>, with the only difference you have to type less. 32-bit systems should generally not use sizes larger than 1024 to avoid stack overflow, while 64-bit systems can safely use sizes up to 2048 or 4096. Use with caution on 32-bit systems.
    template<size_t N>
    using str = fxdstr<N>; 

    #if INTPTR_MAX == INT32_MAX
        using str_t = fxdstr<128>; // alias for default string type, with size 128 (32-bit system default)
    #elif INTPTR_MAX == INT64_MAX
        using str_t = fxdstr<256>; // alias for default string type, with size 256 (64-bit system default)
    #else
        using str_t = fxdstr<128>; // fallback to 128 if pointer size is unknown
    #endif


    // --- to_scf_str for arithmetic types ---
    // Specialization for std::string. converts a STL-string to fxdstr
    template<size_t N>
    fxdstr<N> to_scf_str(const std::string& value) {
        fxdstr<N> out;
        size_t n = value.size();
        if (n > N) {
            out.append(value.c_str(), N);
        } else {
            out.append(value.c_str(), n);
        }
        return out;
    }

    // Specialization for const char*. converts a C-string to fxdstr
    template<size_t N>
    fxdstr<N> to_scf_str(const char* value) {
        fxdstr<N> out;
        if (value) {
            size_t n = std::strlen(value);
            if (n > N) {
                out.append(value, N);
            } else {
                out.append(value, n);
            }
        }
        return out;
    }

    // Generic to_scf_str for arithmetic types with improved format handling. converts a value to fxdstr
    template<size_t N, typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    fxdstr<N> to_scf_str(const T& value) {
        fxdstr<N> out;
        char temp[N + 1];
        // Determine format string based on type
        const char* fmt;
        if constexpr (std::is_same_v<T, int>) fmt = "%d";
        else if constexpr (std::is_same_v<T, unsigned int>) fmt = "%u";
        else if constexpr (std::is_same_v<T, long>) fmt = "%ld";
        else if constexpr (std::is_same_v<T, unsigned long>) fmt = "%lu";
        else if constexpr (std::is_same_v<T, long long>) fmt = "%lld";
        else if constexpr (std::is_same_v<T, unsigned long long>) fmt = "%llu";
        else if constexpr (std::is_same_v<T, float>) fmt = "%f";
        else if constexpr (std::is_same_v<T, double>) fmt = "%lf";
        else if constexpr (std::is_same_v<T, long double>) fmt = "%Lf";
        else if constexpr (std::is_same_v<T, char>) fmt = "%c";
        else if constexpr (std::is_same_v<T, unsigned char>) fmt = "%u";
        else if constexpr (std::is_same_v<T, short>) fmt = "%d";
        else if constexpr (std::is_same_v<T, unsigned short>) fmt = "%u";
        else fmt = "%d"; // fallback
        
        int written = std::snprintf(temp, N + 1, fmt, value);
        if (written > 0) {
            out.append(temp, std::min(static_cast<int>(N), written));
        }
        return out;
    }

    // Specialization for bool. converts a boolean to fxdstr
    template<size_t N>
    fxdstr<N> to_scf_str(const bool& value) {
        fxdstr<N> out;
        out.append(value ? "true" : "false");
        return out;
    }

    // Specialization for fxdstr<M> (downsize or upsize)
    template<size_t N, size_t M>
    fxdstr<N> to_scf_str(const fxdstr<M>& value) {
        fxdstr<N> out;
        if (value.size() > N) throw std::length_error("fxdstr::to_scf_str - source too large for target size");
        out.append(value.c_str(), value.size());
        return out;
    }

    // --- Convenience to_strX aliases ---
    template<typename T> str8    to_str8(const T& v)    { return to_scf_str<8>(v); }
    template<typename T> str16   to_str16(const T& v)   { return to_scf_str<16>(v); }
    template<typename T> str32   to_str32(const T& v)   { return to_scf_str<32>(v); }
    template<typename T> str64   to_str64(const T& v)   { return to_scf_str<64>(v); }
    template<typename T> str128  to_str128(const T& v)  { return to_scf_str<128>(v); }
    template<typename T> str256  to_str256(const T& v)  { return to_scf_str<256>(v); }
    template<typename T> str512  to_str512(const T& v)  { return to_scf_str<512>(v); }
    template<typename T> str1024 to_str1024(const T& v) { return to_scf_str<1024>(v); }
    template<typename T> str2048 to_str2048(const T& v) { return to_scf_str<2048>(v); }
    template<typename T> str4096 to_str4096(const T& v) { return to_scf_str<4096>(v); }

    /**
     * @brief Converts a value to a fxdstr with automatic size selection.
     * @tparam T Type of the value to convert.
     * @param value The value to convert.
     * @return fxdstr<256> containing the string representation of value.
     */
    template<typename T>
    fxdstr<256> to_str(const T& value) {
        return to_scf_str<256>(value);
    }

    template<size_t N>
    std::string to_std_str(const fxdstr<N> &scf_str) {
        return std::string(scf_str.c_str(), scf_str.size());
    }
}


// compiler warnings for 32-bit systems

#if INTPTR_MAX == INT32_MAX

template<>
struct [[deprecated("fxdstr<1024> is discouraged on 32-bit systems due to stack limits")]]
fxdstr<1024>;

template<>
struct [[deprecated("fxdstr<2048> is not supported on 32-bit systems")]]
fxdstr<2048>;

template<>
struct [[deprecated("fxdstr<4096> is not supported on 32-bit systems")]]
fxdstr<4096>;

#endif
