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

inline int s_to_i(const char* s_) {
    int result = 0;

    for (size_t i = 0; s_[i] != '\0'; ++i) {
        if (s_[i] < '0' || s_[i] > '9') {
            return -1; 
        }

        int digit = s_[i] - '0';
        result = result * 10 + digit;
    }

    return result;
}

inline unsigned int s_to_ui(const char* s_) {
    unsigned int result = 0;

    for (size_t i = 0; s_[i] != '\0'; ++i) {
        if (s_[i] < '0' || s_[i] > '9') {
            return 0; 
        }

        int digit = s_[i] - '0';
        result = result * 10 + digit;
    }

    return result;
}

inline unsigned long s_to_ul(const char* s_) {
    unsigned long result = 0;

    for (size_t i = 0; s_[i] != '\0'; ++i) {
        if (s_[i] < '0' || s_[i] > '9') {
            return 0; 
        }

        int digit = s_[i] - '0';
        result = result * 10 + digit;
    }

    return result;
}

inline long long s_to_ll(const char* s_) {
    long long result = 0;

    for (size_t i = 0; s_[i] != '\0'; ++i) {
        if (s_[i] < '0' || s_[i] > '9') {
            return 0; 
        }

        int digit = s_[i] - '0';
        result = result * 10 + digit;
    }

    return result;
}

inline unsigned long long s_to_ull(const char* s_) {
    unsigned long long result = 0;

    for (size_t i = 0; s_[i] != '\0'; ++i) {
        if (s_[i] < '0' || s_[i] > '9') {
            return 0; 
        }

        int digit = s_[i] - '0';
        result = result * 10 + digit;
    }

    return result;
}


}