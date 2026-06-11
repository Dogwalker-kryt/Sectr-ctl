/* 
 * SCF IO - Stack C++ Framework IO components
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

// scf_io.hpp
#pragma once
#include <stdio.h>
#include "scf_cutils.hpp"
#include "scf_type_traits.hpp"
#include <charconv>
#include <unistd.h>
#include "config.hpp"
#include "scf_str.hpp"
#include "scf_optional.hpp"
#include "scf_string_view.hpp"
#include "scf_map.hpp"

#ifdef SCF_ALLOW_STL
    #include <string>
#endif

#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    #include <termios.h>
#elif defined(_WIN32)
    #include <windows.h>
#endif

namespace scf {

// ============================================================
// Type trait to detect fxdstr<N>
// ============================================================
template<typename T>
struct is_fixed_string : scf::type_traits::false_type {};

template<size_t N>
struct is_fixed_string<fxdstr<N>> : scf::type_traits::true_type {};

template<size_t N>
struct is_fixed_string<fxdstr<N>&> : scf::type_traits::true_type {};

template<size_t N>
struct is_fixed_string<const fxdstr<N>&> : scf::type_traits::true_type {};

namespace detail {

    // ============================================================
    // to_cstr() — convert supported types into a stack buffer
    // ============================================================

    // char
    inline int to_cstr(char c, char* out, int max) {
        if (max < 2) return 0;
        out[0] = c;
        out[1] = '\0';
        return 1;
    }

    // const char*
    inline int to_cstr(const char* s, char* out, int max) {
        if (!s || max <= 0) return 0;
        int i = 0;
        const int limit = max - 1;
        while (s[i] && i < limit) {
            out[i] = s[i];
            ++i;
        }
        out[i] = '\0';
        return i;
    }

    // char[]
    template<size_t N>
    inline int to_cstr(const char (&s)[N], char* out, int max) {
        return to_cstr(s, N - 1, out, max);
    }

    // char* + size
    inline int to_cstr(const char* s, size_t len, char* out, int max) {
        if (!s || max <= 0) return 0;
        int i = 0;
        const int limit = max - 1;
        while (i < static_cast<int>(len) && i < limit) {
            out[i] = s[i];
            ++i;
        }
        out[i] = '\0';
        return i;
    }

    #ifdef SCF_ALLOW_STL
    // std::string_view (if you still want to support it)
    inline int to_cstr(std::string_view s, char* out, int max) {
        if (max <= 0) return 0;
        int i = 0;
        const int limit = max - 1;
        const int n = static_cast<int>(s.size());
        while (i < limit && i < n) {
            out[i] = s[static_cast<size_t>(i)];
            ++i;
        }
        out[i] = '\0';
        return i;
    }
    #endif

    // fxdstr<N>
    template<size_t N>
    inline int to_cstr(const fxdstr<N>& s, char* out, int max) {
        return to_cstr(s.data(), s.size(), out, max);
    }

    // str_view
    inline int to_cstr(str_view s, char* out, int max) {
        return to_cstr(s.data(), s.size(), out, max);
    }

    // scf::optional
    template<typename T>
    inline int to_cstr(const scf::optional<T>& opt, char* out, int max) {
        if (!opt.has_value()) {
            return to_cstr("nullopt", out, max);
        }
        return to_cstr(*opt, out, max);
    }

    #ifdef SCF_ALLOW_STL
        // std::string
        inline int to_cstr(const std::string& s, char* out, int max) {
            return to_cstr(s.c_str(), out, max);
        }
    #endif

    // Signed integers
    template<typename T>
    inline int to_cstr_signed(T value, char* out, int max) {
        static_assert(scf::type_traits::is_integral_v<T> && scf::type_traits::is_signed_v<T>);
        if (max <= 0) return 0;
        auto first = out;
        auto last  = out + max - 1;
        auto res   = std::to_chars(first, last, value);
        if (res.ec != std::errc{}) {
            out[0] = '\0';
            return 0;
        }
        *res.ptr = '\0';
        return static_cast<int>(res.ptr - out);
    }

    // Unsigned integers
    template<typename T>
    inline int to_cstr_unsigned(T value, char* out, int max) {
        static_assert(scf::type_traits::is_integral_v<T> && scf::type_traits::is_unsigned_v<T>);
        if (max <= 0) return 0;
        auto first = out;
        auto last  = out + max - 1;
        auto res   = std::to_chars(first, last, value);
        if (res.ec != std::errc{}) {
            out[0] = '\0';
            return 0;
        }
        *res.ptr = '\0';
        return static_cast<int>(res.ptr - out);
    }

    // Floating point
    template<typename T>
    inline int to_cstr_floating(T value, char* out, int max) {
        static_assert(scf::type_traits::is_floating_point_v<T>);
        if (max <= 0) return 0;
        int written = snprintf(out, max, "%g", value);
        if (written < 0 || written >= max) {
            out[0] = '\0';
            return 0;
        }
        return written;
    }

    // Dispatcher
    template<typename T>
    inline int to_cstr(const T& v, char* out, int max) {
        if constexpr (scf::type_traits::is_same_v<T, char>) {
            return to_cstr(v, out, max);
        }
        else if constexpr (scf::type_traits::is_same_v<T, const char*>) {
            return to_cstr(v, out, max);
        }
        else if constexpr (scf::type_traits::is_same_v<T, char*>) {
            return to_cstr(v, out, max);
        }
        else if constexpr (scf::type_traits::is_array_v<T>) {
            return to_cstr(v, out, max);
        }
        else if constexpr (scf::is_fixed_string<T>::value) {
            return to_cstr(v, out, max);
        }
        else if constexpr (scf::type_traits::is_same_v<T, str_view>) {
            return to_cstr(v, out, max);
        }
        else if constexpr (scf::type_traits::is_integral_v<T> && scf::type_traits::is_signed_v<T>) {
            return to_cstr_signed(v, out, max);
        }
        else if constexpr (scf::type_traits::is_integral_v<T> && scf::type_traits::is_unsigned_v<T>) {
            return to_cstr_unsigned(v, out, max);
        }
        else if constexpr (scf::type_traits::is_floating_point_v<T>) {
            return to_cstr_floating(v, out, max);
        }
        else if constexpr (scf::is_optional<T>::value) {
            return to_cstr(v, out, max);
        }
        else if constexpr (scf::type_traits::is_pointer_v<T>) {
            if (!v) {
                return to_cstr("[SCF] nullptr", out, max);
            }
            return to_cstr(*v, out, max);
        }
        #ifdef SCF_ALLOW_STL
            else if constexpr (scf::type_traits::is_same_v<T, std::string>) {
                return to_cstr(v.c_str(), out, max);
            }
        #endif
        else {
            static_assert(!sizeof(T), "Unsupported type for to_cstr()");
        }
    }

    // ============================================================
    // Raw line reader — getline-like, no leftovers
    // ============================================================
    inline bool read_line_raw(char* buffer, size_t max) {
        if (max == 0) return false;
        if (!fgets(buffer, static_cast<int>(max), stdin))
            return false;

        size_t len = strlen(buffer);

        // If no newline, discard rest of line
        if (len > 0 && buffer[len - 1] != '\n') {
            int c;
            while ((c = getc(stdin)) != '\n' && c != EOF) {}
        }

        // Strip trailing '\n' and optional '\r'
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[--len] = '\0';
        }
        if (len > 0 && buffer[len - 1] == '\r') {
            buffer[--len] = '\0';
        }

        return true;
    }

} // namespace detail

// ============================================================
// Printing
// ============================================================

// Custom nullopt string
inline const char* nullopt_str = "nullopt";

// better
template<typename... Args>
inline void vprint(size_t N, FILE* out, bool leading_ln, bool trailing_ln, bool flush, const Args&... args) {
    char buf[N];

    if (leading_ln)
        fputc('\n', out);

    ((
        detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
        fputs(buf, out)
    ), ...);

    if (trailing_ln)
        fputc('\n', out);

    if (flush)
        fflush(out);
}

template<size_t N = 1024, typename... Args>
inline void print(const Args&... args) {
    vprint(N, stdout, false, false, false, args...);
}

template<size_t N = 1024, typename... Args>
inline void println(const Args&... args) {
    vprint(N, stdout, false, true, false, args...);
}

template<size_t N = 1024, typename... Args>
inline void lnprintln(const Args&... args) {
    vprint(N, stdout, true, true, false, args...);
}

template<size_t N = 1024, typename... Args>
inline void print_cerr(const Args&... args) {
    vprint(N, stderr, false, false, false, args...);
}

template<size_t N = 1024, typename... Args>
inline void println_cerr(const Args&... args) {
    vprint(N, stderr, false, true, false, args...);
}

template<size_t N = 1024, typename... Args>
inline void lnprintln_cerr(const Args&... args) {
    vprint(N, stderr, true, true, false, args...);
}

// Flushing versions
template<size_t N = 1024, typename... Args>
inline void print_flush(const Args&... args) {
    vprint(N, stdout, false, false, true, args...);
}

template<size_t N = 1024, typename... Args>
inline void println_flush(const Args&... args) {
    vprint(N, stdout, false, true, true, args...);
}

template<size_t N = 1024, typename... Args>
inline void lnprintln_flush(const Args&... args) {
    vprint(N, stdout, true, true, true, args...);
}

template<size_t N = 1024, typename... Args>
inline void print_cerr_flush(const Args&... args) {
    vprint(N, stderr, false, false, true, args...);
}

template<size_t N = 1024, typename... Args>
inline void println_cerr_flush(const Args&... args) {
    vprint(N, stderr, false, true, true, args...);
}

template<size_t N = 1024, typename... Args>
inline void lnprintln_cerr_flush(const Args&... args) {
    vprint(N, stderr, true, true, true, args...);
}

// ============================================================
// Reading
// ============================================================

inline void flush_stdin() {
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    tcflush(STDIN_FILENO, TCIFLUSH);
#elif defined(_WIN32)
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif
}

// Numeric read: returns bool for success/failure
template<typename T>
bool read(T& out) {
    static_assert(scf::type_traits::is_arithmetic_v<T>, "read<T> only supports numeric types");

    char buffer[1024];
    if (!detail::read_line_raw(buffer, sizeof(buffer)))
        return false;

    const char* first = buffer;
    const char* last  = buffer + strlen(buffer);

    if constexpr (scf::type_traits::is_integral_v<T>) {
        auto res = std::from_chars(first, last, out);
        return res.ec == std::errc{} && res.ptr == last;
    } else if constexpr (scf::type_traits::is_floating_point_v<T>) {
        char* end = nullptr;
        if constexpr (scf::type_traits::is_same_v<T, float>) {
            float v = strtof(first, &end);
            if (end == first) return false;
            out = v;
            return true;
        } else {
            double v = strtod(first, &end);
            if (end == first) return false;
            out = static_cast<T>(v);
            return true;
        }
    }
    return false;
}

// fxdstr read: getline-like, truncates if too small
template<size_t N>
bool read(fxdstr<N>& out) {
    char buffer[N];
    if (!detail::read_line_raw(buffer, N))
        return false;

    size_t len = 0;
    while (buffer[len] != '\0' && len < N - 1) {
        out.data()[len] = buffer[len];
        ++len;
    }

    out.set_length(len);
    return true;
}

// char[] read
template<size_t N>
bool read(char (&out)[N]) {
    return detail::read_line_raw(out, N);
}

// char* read
inline bool read(char* out, size_t max) {
    return detail::read_line_raw(out, max);
}

// Read input. doesnt flush
template<typename T>
T read() {
    T v{};
    read(v);
    return v;
}

// Flushes input buffer before reading to avoid leftover input issues
template<typename T>
T readflsh() {
    T v{};
    flush_stdin();
    read(v);
    return v;
}

// ============================================================
// to_cstr for static_map and static_unordered_map
// ============================================================
template<typename K, typename V, size_t N>
inline int to_cstr(const scf::static_map<K, V, N>& map, char* out, int max) {
    int written = 0;
    written += snprintf(out + written, max - written, "static_map{");
    for (size_t i = 0; i < map.size(); ++i) {
        if (i > 0) written += snprintf(out + written, max - written, ", ");
        written += detail::to_cstr(map.data()[i].first, out + written, max - written);
        written += snprintf(out + written, max - written, ": ");
        written += detail::to_cstr(map.data()[i].second, out + written, max - written);
    }
    written += snprintf(out + written, max - written, "}");
    return written;
}

template<typename K, typename V, size_t N>
inline int to_cstr(const scf::static_unordered_map<K, V, N>& map, char* out, int max) {
    int written = 0;
    written += snprintf(out + written, max - written, "static_unordered_map{");
    for (size_t i = 0; i < map.size(); ++i) {
        if (i > 0) written += snprintf(out + written, max - written, ", ");
        written += detail::to_cstr(map.data()[i].first, out + written, max - written);
        written += snprintf(out + written, max - written, ": ");
        written += detail::to_cstr(map.data()[i].second, out + written, max - written);
    }
    written += snprintf(out + written, max - written, "}");
    return written;
}

} // namespace scf
