#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        return ++count_;
    }
    size_t DecRef() {
        return --count_;
    }
    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    void IncRef() {
        counter_.IncRef();
    }

    void DecRef() {
        counter_.DecRef();
        if (counter_.RefCount() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    size_t RefCount() const {
        return counter_.RefCount();
    }

    RefCounted& operator=(RefCounted<Derived, SimpleCounter, Deleter>&& other) {
        return *this;
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    IntrusivePtr() {
        ptr_ = nullptr;
    }
    IntrusivePtr(std::nullptr_t) {
        ptr_ = nullptr;
    }
    IntrusivePtr(T* ptr) {
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) {
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    IntrusivePtr(IntrusivePtr&& other) {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (this == &other) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (this == &other) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        return *this;
    }
    template <typename Y>
    IntrusivePtr& operator=(Y* ptr) {
        Reset();
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    ~IntrusivePtr() {
        Reset();
    }

    void Reset() {
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        if (ptr == ptr_) {
            return;
        }
        Reset();
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (!ptr_) {
            return 0;
        }
        return ptr_->RefCount();
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
