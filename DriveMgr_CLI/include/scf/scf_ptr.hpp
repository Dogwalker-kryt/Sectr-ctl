#include "scf_cutils.hpp"

namespace scf {

template <typename T>
class stack_ptr {
    alignas(T) char storage_[sizeof(T)]; 
    bool initialized_ = false;

public:

    stack_ptr() = default;

    template <typename... Args>
    explicit stack_ptr(Args&&... args) {
        new (storage_) T(scf::type_traits::forward<Args>(args)...);
        initialized_ = true;
    }

    ~stack_ptr() {
        if (initialized_) {
            reinterpret_cast<T*>(storage_)->~T();
        }
    }

    stack_ptr(const stack_ptr&) = delete;
    stack_ptr& operator=(const stack_ptr&) = delete;

    T& operator*() {
        return *reinterpret_cast<T*>(storage_);
    }
    const T& operator*() const {
        return *reinterpret_cast<const T*>(storage_);
    }
    T* operator->() {
        return reinterpret_cast<T*>(storage_);
    }
    const T* operator->() const {
        return reinterpret_cast<const T*>(storage_);
    }

    explicit operator bool() const {
        return initialized_;
    }
};

}