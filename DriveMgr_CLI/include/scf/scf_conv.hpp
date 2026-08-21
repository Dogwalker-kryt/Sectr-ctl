#pragma once

#include "scf_cutils.hpp"

namespace scf {

inline int c_to_asciiv(char c_) {
    return c_;
}

inline int c_to_i(char c_) {
    if (c_ >= '0' && c_ <= '9') {
        return static_cast<int>(c_ - '0');
    }
    return 0;
}

inline unsigned int c_to_ui(char c_) {
    if (c_ >= '0' && c_ <= '9') {
        return static_cast<unsigned int>(c_ - '0');
    }
    return 0;
}

inline unsigned long c_to_ul(char c_) {
    if (c_ >= '0' && c_ <= '9') {
        return static_cast<unsigned long>(c_ - '0');
    }
    return 0;
}

inline long long c_to_ll(char c_) {
    if (c_ >= '0' && c_ <= '9') {
        return static_cast<long long>(c_ - '0');
    }
    return 0;
}

inline unsigned long long c_to_ull(char c_) {
    if (c_ >= '0' && c_ <= '9') {
        return static_cast<unsigned long long>(c_ - '0');
    }
    return 0;
}

template<typename T>
inline bool s_to_integral(const char* s_, T& out) {
    if (!s_) return false;

    const char* end = s_;
    while (*end != '\0') ++end;

    return scf::cstr_to_integral(s_, end, out);
}

inline bool s_to_i(const char* s_, int& out) {
    return s_to_integral(s_, out);
}

inline int s_to_i(const char* s_) {
    int result = 0;
    return s_to_i(s_, result) ? result : -1;
}

inline bool s_to_ui(const char* s_, unsigned int& out) {
    return s_to_integral(s_, out);
}

inline unsigned int s_to_ui(const char* s_) {
    unsigned int result = 0;
    return s_to_ui(s_, result) ? result : 0;
}

inline bool s_to_l(const char* s_, long& out) {
    return s_to_integral(s_, out);
}

inline long s_to_l(const char* s_) {
    long result = 0;
    return s_to_l(s_, result) ? result : 0;
}

inline bool s_to_ul(const char* s_, unsigned long& out) {
    return s_to_integral(s_, out);
}

inline unsigned long s_to_ul(const char* s_) {
    unsigned long result = 0;
    return s_to_ul(s_, result) ? result : 0;
}

inline bool s_to_ll(const char* s_, long long& out) {
    return s_to_integral(s_, out);
}

inline long long s_to_ll(const char* s_) {
    long long result = 0;
    return s_to_ll(s_, result) ? result : 0;
}

inline bool s_to_ull(const char* s_, unsigned long long& out) {
    return s_to_integral(s_, out);
}

inline unsigned long long s_to_ull(const char* s_) {
    unsigned long long result = 0;
    return s_to_ull(s_, result) ? result : 0;
}


}