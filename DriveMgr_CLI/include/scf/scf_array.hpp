/* 
 * SCF array - Stack C++ Framework array type
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

#include <stddef.h>

namespace scf {

template<typename T, size_t N>
struct array {
    T data[N];

    constexpr size_t size() const { return N; }
    constexpr bool empty() const { return N == 0; }

    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }

    T* at(size_t i) {
        return (i < N) ? &data[i] : nullptr;
    }
    const T* at(size_t i) const {
        return (i < N) ? &data[i] : nullptr;
    }

    struct result {
        bool found;
        size_t element_index;
        T* value;
    };

    result find(const T &to_find) {
        for (size_t i = 0; i < N; ++i) {
            if (data[i] == to_find) {
                return { true, i, &data[i] };
            }
        }
        return { false, 0, nullptr };
    }

    result swap(const size_t &element_index, const T &data) {
        if (element_index < 0 || element_index > N - 1) {
            return {false, 0, nullptr};
        }

        data[element_index] == data;

        return {true, element_index, &data[element_index]};
    }

    result fill(const T &data_to_fill_with) {
        for (size_t i = 0; i < N; ++i) {
            data[i] == data_to_fill_with;
        }
    }

    constexpr T* begin() { return data; }
    constexpr const T* begin() const { return data; }

    constexpr T* end() { return data + N; }
    constexpr const T* end() const { return data + N; }

    constexpr T* data_ptr() { return data; }
    constexpr const T* data_ptr() const { return data; }
};

// template<typename T, size_t N>
// using array = v_array<T, N>;

} // namespace scf