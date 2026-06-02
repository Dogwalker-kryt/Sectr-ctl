#pragma once

#include <array>
#include <iostream>
#include <stdexcept>

namespace scf {

template<typename K, typename V, size_t N>
class static_map {
    std::array<std::pair<K, V>, N> data;
    size_t count = 0;
    
public:
    static_map(std::initializer_list<std::pair<K, V>> init) {
        for (const auto& [k, v] : init) {
            insert(k, v);
        }
    }

    static constexpr size_t npos = static_cast<size_t>(-1);

    // Find by key → pointer to value (or nullptr); O(log n)
    V* find_key(const K& key) {
        int left = 0;
        int right = count - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;

            if (data[mid].first == key) {

                return &data[mid].second;

            } else if (data[mid].first < key) {

                left = mid + 1;
            } else {

                right =  mid - 1;

            }

        }
        return nullptr;
    }
    // const V* find_key(const K& key) const;

    // returns sizeof(size_t) if not found; O(log n)
    size_t find_key_idx(const K& key) const {
        int left = 0;
        int right = count - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;

            if (data[mid].first == key) {

                return mid;

            } else if (data[mid].first < key) {

                left = mid + 1;
            } else {

                right =  mid - 1;

            }

        }
        return npos;
    }


    // Check if key exists
    bool contains(const K& key) const {
        int left = 0;
        int right = count - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;

            if (data[mid].first == key) {
                return true;
            } else if (data[mid].first < key) {
                left = mid + 1;
            } else {
                right =  mid - 1;
            }

        }
        return false;
    }
    
// Find key by value; O(n) linear search
    K* find_value(const V& value) {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].second == value) {
                return &data[i].first;
            }
        }
        return nullptr;
    }

    const K* find_value(const V& value) const {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].second == value) {
                return &data[i].first;
            }
        }
        return nullptr;
    }
    
    // Insert → throws if full or key already exists; maintains sorted order; O(n)
    void insert(const K& key, const V& value) {
        if (count >= N) {
            throw std::length_error("static_map is full");
        }

        // Check if key already exists
        if (contains(key)) {
            throw std::invalid_argument("Key already exists in static_map");
        }

        // Find insertion point (maintain sorted order by key)
        size_t insert_pos = 0;
        for (size_t i = 0; i < count; ++i) {
            if (data[i].first < key) {
                insert_pos = i + 1;
            } else {
                break;
            }
        }

        // Shift elements to the right
        for (size_t i = count; i > insert_pos; --i) {
            data[i] = data[i - 1];
        }

        // Insert new element
        data[insert_pos] = {key, value};
        ++count;
    }
    
    // Erase by key → returns true if erased, false if not found; O(n)
    bool erase_entry(const K& key) {
        size_t idx = find_key_idx(key);
        
        if (idx == npos) {
            return false;
        }

        // Shift elements to the left
        for (size_t i = idx; i < count - 1; ++i) {
            data[i] = data[i + 1];
        }

        --count;
        return true;
    }
    
    // Capacity / size info
    size_t size() const { return count; }
    constexpr size_t capacity() const { return N; }
    bool full() const { return count >= N; }
    bool empty() const { return count == 0; }
};


// ============================================================
// static_unordered_map — unsorted, O(1) insert, O(n) find
// ============================================================

template<typename K, typename V, size_t N>
class static_unordered_map {
    std::array<std::pair<K, V>, N> data;
    size_t count = 0;
    
public:
    static_unordered_map(std::initializer_list<std::pair<const K, V>> init) {
        for (const auto& [k, v] : init) {
            emplace(k, v);
        }
    }

    static constexpr size_t npos = static_cast<size_t>(-1);

    // Find by key → pointer to value (or nullptr); O(n)
    V* find_key(const K& key) {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].first == key) {
                return &data[i].second;
            }
        }
        return nullptr;
    }

    const V* find_key(const K& key) const {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].first == key) {
                return &data[i].second;
            }
        }
        return nullptr;
    }

    // Returns index if found, npos otherwise; O(n)
    size_t find_key_idx(const K& key) const {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].first == key) {
                return i;
            }
        }
        return npos;
    }

    // Check if key exists; O(n)
    bool contains(const K& key) const {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].first == key) {
                return true;
            }
        }
        return false;
    }

    // Find key by value; O(n)
    K* find_value(const V& value) {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].second == value) {
                return &data[i].first;
            }
        }
        return nullptr;
    }

    const K* find_value(const V& value) const {
        for (size_t i = 0; i < count; ++i) {
            if (data[i].second == value) {
                return &data[i].first;
            }
        }
        return nullptr;
    }

    // Insert → throws if full or key exists; O(1) amortized
    void insert(const K& key, const V& value) {
        if (count >= N) {
            throw std::length_error("static_unordered_map is full");
        }

        if (contains(key)) {
            throw std::invalid_argument("Key already exists in static_unordered_map");
        }

        data[count] = {key, value};
        ++count;
    }

    // Erase by key → returns true if erased, false if not found; O(n)
    bool erase_entry(const K& key) {
        size_t idx = find_key_idx(key);

        if (idx == npos) {
            return false;
        }

        // Move last element to erased position (unordered, so order doesn't matter)
        data[idx] = data[count - 1];
        --count;
        return true;
    }

    // Capacity / size info
    size_t size() const { return count; }
    constexpr size_t capacity() const { return N; }
    bool full() const { return count >= N; }
    bool empty() const { return count == 0; }
};


}