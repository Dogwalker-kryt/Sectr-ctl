/*
 * SCF map - Stack C++ Framework static_map/static_unordered_map type
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
#include "scf_array.hpp"
#include "scf_cutils.hpp"
#include "scf_type_traits.hpp"
#include "config.hpp"

#ifdef SCF_ALLOW_STL
#include <type_traits>
#endif

namespace scf {

// Custom pair to avoid std::pair
template<typename K, typename V>
struct pair {
    K first;
    V second;
    pair() = default;
    pair(const K& k, const V& v) : first(k), second(v) {}
    pair(K&& k, V&& v) : first(scf::move(k)), second(scf::move(v)) {}
    pair& operator=(const pair&) = default;
    pair& operator=(pair&&) = default;
};

template<typename K, typename V>
pair(K, V) -> pair<K, V>;

// ============================================================
// static_map — sorted, O(log n) find, O(n) insert
// ============================================================

template<typename K, typename V, size_t N>
class static_map {
    scf::array<pair<K, V>, N> data_;
    size_t count_ = 0;

public:
    static_map(scf::type_traits::initializer_list<pair<K, V>> init) {
        for (const auto& [k, v] : init) {
            insert(k, v);
        }
    }

    static constexpr size_t npos = static_cast<size_t>(-1);

    // Find by key → pointer to value (or nullptr); O(log n)
    V* find_key(const K& key) {
        int left = 0;
        int right = static_cast<int>(count_) - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (data_[mid].first == key) {
                return &data_[mid].second;
            } else if (data_[mid].first < key) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return nullptr;
    }

    const V* find_key(const K& key) const {
        int left = 0;
        int right = static_cast<int>(count_) - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (data_[mid].first == key) {
                return &data_[mid].second;
            } else if (data_[mid].first < key) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return nullptr;
    }

    V* find_key(str_view key) {
        return find_key(K(key));
    }

    const V* find_key(str_view key) const {
        return find_key(K(key));
    }

    // returns npos if not found; O(log n)
    size_t find_key_idx(const K& key) const {
        int left = 0;
        int right = static_cast<int>(count_) - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (data_[mid].first == key) {
                return mid;
            } else if (data_[mid].first < key) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return npos;
    }

    size_t find_key_idx(str_view key) const {
        return find_key_idx(K(key));
    }

    // Check if key exists
    bool contains(const K& key) const {
        return find_key_idx(key) != npos;
    }

    bool contains(str_view key) const {
        return contains(K(key));
    }

    // Find key by value; O(n) linear search
    K* find_value(const V& value) {
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].second == value) {
                return &data_[i].first;
            }
        }
        return nullptr;
    }

    const K* find_value(const V& value) const {
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].second == value) {
                return &data_[i].first;
            }
        }
        return nullptr;
    }

    // Insert → throws if full or key already exists; maintains sorted order; O(n)
    void insert(const K& key, const V& value) {
        if (count_ >= N) {
            throw length_error("static_map is full");
        }
        if (contains(key)) {
            throw invalid_argument("Key already exists in static_map");
        }

        size_t insert_pos = 0;
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].first < key) {
                insert_pos = i + 1;
            } else {
                break;
            }
        }

        for (size_t i = count_; i > insert_pos; --i) {
            data_[i] = data_[i - 1];
        }

        data_[insert_pos] = {key, value};
        ++count_;
    }

    void insert(str_view key, const V& value) {
        insert(K(key), value);
    }

    // Erase by key → returns true if erased, false if not found; O(n)
    bool erase_entry(const K& key) {
        size_t idx = find_key_idx(key);
        if (idx == npos) {
            return false;
        }

        for (size_t i = idx; i < count_ - 1; ++i) {
            data_[i] = data_[i + 1];
        }

        --count_;
        return true;
    }

    bool erase_entry(str_view key) {
        return erase_entry(K(key));
    }

    // Capacity / size info
    size_t size() const { return count_; }
    constexpr size_t capacity() const { return N; }
    bool full() const { return count_ >= N; }
    bool empty() const { return count_ == 0; }

    // Direct access (use with care)
    const scf::array<pair<K, V>, N>& data() const { return data_; }
};

// ============================================================
// static_unordered_map — unsorted, O(1) insert, O(n) find
// ============================================================

template<typename K, typename V, size_t N>
class static_unordered_map {
    scf::array<pair<K, V>, N> data_;
    size_t count_ = 0;

public:
#ifdef SCF_ALLOW_STL
    static_unordered_map(std::initializer_list<std::pair<K, V>> init) {
        for (const auto& [k, v] : init) {
            emplace(k, v);
        }
    }
#endif

    static_unordered_map(scf::type_traits::initializer_list<pair<K, V>> init) {
        for (const auto& [k, v] : init) {
            emplace(k, v);
        }
    }

    static constexpr size_t npos = static_cast<size_t>(-1);

    // Find by key → pointer to value (or nullptr); O(n)
    V* find_key(const K& key) {
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].first == key) {
                return &data_[i].second;
            }
        }
        return nullptr;
    }

    const V* find_key(const K& key) const {
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].first == key) {
                return &data_[i].second;
            }
        }
        return nullptr;
    }

    V* find_key(str_view key) {
        return find_key(K(key));
    }

    const V* find_key(str_view key) const {
        return find_key(K(key));
    }

    // Returns index if found, npos otherwise; O(n)
    size_t find_key_idx(const K& key) const {
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].first == key) {
                return i;
            }
        }
        return npos;
    }

    size_t find_key_idx(str_view key) const {
        return find_key_idx(K(key));
    }

    // Check if key exists; O(n)
    bool contains(const K& key) const {
        return find_key_idx(key) != npos;
    }

    bool contains(str_view key) const {
        return contains(K(key));
    }

    // Find key by value; O(n)
    K* find_value(const V& value) {
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].second == value) {
                return &data_[i].first;
            }
        }
        return nullptr;
    }

    const K* find_value(const V& value) const {
        for (size_t i = 0; i < count_; ++i) {
            if (data_[i].second == value) {
                return &data_[i].first;
            }
        }
        return nullptr;
    }

    // Insert → throws if full or key exists; O(1) amortized
    void insert(const K& key, const V& value) {
        if (count_ >= N) {
            throw length_error("static_unordered_map is full");
        }
        if (contains(key)) {
            throw invalid_argument("Key already exists in static_unordered_map");
        }
        data_[count_] = {key, value};
        ++count_;
    }

    void insert(str_view key, const V& value) {
        insert(K(key), value);
    }

    void emplace(const K& key, const V& value) {
        insert(key, value);
    }

    // Erase by key → returns true if erased, false if not found; O(n)
    bool erase_entry(const K& key) {
        size_t idx = find_key_idx(key);
        if (idx == npos) {
            return false;
        }
        data_[idx] = data_[count_ - 1];
        --count_;
        return true;
    }

    bool erase_entry(str_view key) {
        return erase_entry(K(key));
    }

    // Capacity / size info
    size_t size() const { return count_; }
    constexpr size_t capacity() const { return N; }
    bool full() const { return count_ >= N; }
    bool empty() const { return count_ == 0; }

    // Iterators
    auto begin() { return data_.begin(); }
    auto end() { return data_.begin() + count_; }
    auto begin() const { return data_.begin(); }
    auto end() const { return data_.begin() + count_; }

    // Direct access (use with care)
    const scf::array<pair<K, V>, N>& data() const { return data_; }
};

} // namespace scf