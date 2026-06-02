#pragma once
#include <cstdio>
#include <type_traits>
#include <string_view>
#include <charconv>
#include <cstring>
#include <unistd.h>
#include "scf_str.hpp"

#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    #include <termios.h>
#elif defined(_WIN32)
    #include <windows.h>
#endif

namespace scf {

namespace io {

    // ============================================================
    // Type trait to detect fxdstr<N>
    // ============================================================
    template<typename T>
    struct is_fixed_string : std::false_type {};

    template<size_t N>
    struct is_fixed_string<fxdstr<N>> : std::true_type {};

    template<size_t N>
    struct is_fixed_string<fxdstr<N>&> : std::true_type {};

    template<size_t N>
    struct is_fixed_string<const fxdstr<N>&> : std::true_type {};

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
            while (s[i] && s[i] != '\r' && i < limit) {
                out[i] = s[i];
                ++i;
            }
            out[i] = '\0';
            return i;
        }

        // std::string_view
        inline int to_cstr(std::string_view s, char* out, int max) {
            if (max <= 0) return 0;
            int i = 0;
            const int limit = max - 1;
            const int n = static_cast<int>(s.size());
            while (i < limit && i < n) {
                char c = s[static_cast<size_t>(i)];
                if (c == '\r') break;
                out[i] = c;
                ++i;
            }
            out[i] = '\0';
            return i;
        }

        // std::string
        inline int to_cstr(const std::string& s, char* out, int max) {
            return to_cstr(static_cast<std::string_view>(s), out, max);
        }

        // fxdstr<N>
        template<size_t N>
        inline int to_cstr(const fxdstr<N>& s, char* out, int max) {
            return to_cstr(std::string_view{s.data(), s.length()}, out, max);
        }

        template<typename T>
        inline int to_cstr_signed(T value, char* out, int max) {
            static_assert(std::is_integral_v<T> && std::is_signed_v<T>);
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

        // --- Unsigned integers ---
        template<typename T>
        inline int to_cstr_unsigned(T value, char* out, int max) {
            static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
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

        // Dispatcher
        template<typename T>
        inline int to_cstr(const T& v, char* out, int max) {
            if constexpr (std::is_pointer_v<T>) {
                // Handle pointer types
                if (!v) {
                    // Print "nullptr"
                    if (max < 8) return 0;
                    const char* null_str = "nullptr";
                    int i = 0;
                    while (null_str[i] && i < max - 1) {
                        out[i] = null_str[i];
                        ++i;
                    }
                    out[i] = '\0';
                    return i;
                }
                // Dereference and delegate to dispatcher
                return to_cstr(*v, out, max);
            }
            else if constexpr (std::is_same_v<T, char>)
                return to_cstr(v, out, max);

            else if constexpr (std::is_same_v<T, std::string>)
                return to_cstr(static_cast<std::string_view>(v), out, max);

            else if constexpr (std::is_convertible_v<T, const char*>)
                return to_cstr(static_cast<const char*>(v), out, max);

            else if constexpr (std::is_same_v<T, std::string_view>)
                return to_cstr(v, out, max);

            else if constexpr (is_fixed_string<T>::value)
                return to_cstr(v, out, max);

            else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
                return to_cstr_signed(v, out, max);

            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
                return to_cstr_unsigned(v, out, max);

            else if constexpr (std::is_floating_point_v<T>)
                return to_cstr(v, out, max);

            else
                static_assert(!sizeof(T), "Unsupported type for print()");
        }

        // ============================================================
        // Raw line reader — getline-like, no leftovers
        // ============================================================
        inline bool read_line_raw(char* buffer, size_t max) {
            if (max == 0) return false;
            if (!std::fgets(buffer, static_cast<int>(max), stdin))
                return false;

            size_t len = std::strlen(buffer);

            // If no newline, discard rest of line
            if (len > 0 && buffer[len - 1] != '\n') {
                int c;
                while ((c = std::getc(stdin)) != '\n' && c != EOF) {}
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

    // standard printing to stdout and stderr with no flushing

    template<typename... Args>
    void print(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stdout)
        ), ...);
    }

    template<typename... Args>
    void println(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stdout)
        ), ...);

        std::fputc('\n', stdout);
    }

    template<typename... Args>
    void print_cerr(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stderr)
        ), ...);
    }

    template<typename... Args>
    void println_cerr(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stderr)
        ), ...);

        std::fputc('\n', stderr);
    }

    // flushing

    inline void flush_stdout() {
        fflush(stdout);
    }

    inline void flush_stderr() {
        fflush(stderr);
    }

    // standard printing to stdout and stderr with flushing

    template<typename... Args>
    void print_flush(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stdout)
        ), ...);

        flush_stdout();
    }

    template<typename... Args>
    void println_flush(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stdout)
        ), ...);

        std::fputc('\n', stdout);
        flush_stdout();
    }

    template<typename... Args>
    void print_cerr_flush(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stderr)
        ), ...);

        flush_stderr();
    }

    template<typename... Args>
    void println_cerr_flush(const Args&... args) {
        char buf[1024];

        ((
            detail::to_cstr(args, buf, static_cast<int>(sizeof(buf))),
            std::fputs(buf, stderr)
        ), ...);

        std::fputc('\n', stderr);

        flush_stderr();
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
        static_assert(std::is_arithmetic_v<T>, "read<T> only supports numeric types");

        char buffer[1024];
        if (!detail::read_line_raw(buffer, sizeof(buffer)))
            return false;

        const char* first = buffer;
        const char* last  = buffer + std::strlen(buffer);

        if constexpr (std::is_integral_v<T>) {
            auto res = std::from_chars(first, last, out);
            return res.ec == std::errc{} && res.ptr == last;

        } else if constexpr (std::is_floating_point_v<T>) {
            // Simple strtod-based parse
            char* end = nullptr;

            if constexpr (std::is_same_v<T, float>) {

                float v = std::strtof(first, &end);
                if (end == first) return false;
                out = v;
                return true;

            } else {

                double v = std::strtod(first, &end);
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
} // namespace scf
}