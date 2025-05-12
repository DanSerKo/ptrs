#pragma once

#include "sw_fwd.h"

#include <cstddef>

class ControlBlock {
public:
    ControlBlock() {
        cnt_ = 1;
    }
    virtual ~ControlBlock() = default;

    void Add() {
        cnt_++;
    }

    void Del() {
        cnt_--;
        if (cnt_ == 0) {
            OnZero();
        }
    }
    virtual void OnZero() = 0;

    size_t GetCnt() const {
        return cnt_;
    }

private:
    size_t cnt_;
};

template <typename T>
class PointingControlBlock : public ControlBlock {
public:
    PointingControlBlock(T* ptr) : ControlBlock(), ptr_(ptr) {
    }
    virtual ~PointingControlBlock() = default;
    void OnZero() override {
        if (ptr_) {
            delete ptr_;
        }
        ptr_ = nullptr;
        delete this;
    }

private:
    T* ptr_;
};

template <typename T>
class EmplacingControlBlock : public ControlBlock {
public:
    template <typename... Args>
    EmplacingControlBlock(Args&&... args) : ControlBlock() {
        new (&buffer_) T(std::forward<Args>(args)...);
    }
    virtual ~EmplacingControlBlock() = default;

    void OnZero() override {
        Get()->~T();
        delete this;
    }

    T* Get() {
        return reinterpret_cast<T*>(&buffer_);
    }

private:
    alignas(T) std::byte buffer_[sizeof(T)];
};

template <typename T>
class SharedPtr {
public:
    SharedPtr() {
        block_ = nullptr;
        ptr_ = nullptr;
    }
    SharedPtr(std::nullptr_t) {
        block_ = nullptr;
        ptr_ = nullptr;
    }

    template <typename Y>
    SharedPtr(Y* ptr) {
        block_ = new PointingControlBlock<Y>(ptr);
        ptr_ = ptr;
    }

    SharedPtr(const SharedPtr& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->Add();
        }
    }
    SharedPtr(SharedPtr&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->Add();
        }
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        block_ = other.block_;
        ptr_ = ptr;
        if (block_) {
            block_->Add();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->Add();
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        if (this == &other) {
            return *this;
        }
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->Add();
        }
        return *this;
    }
    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ~SharedPtr() {
        Reset();
    }

    void Reset() {
        if (block_) {
            block_->Del();
        }
        block_ = nullptr;
        ptr_ = nullptr;
    }

    template <typename Y>
    void Reset(Y* ptr) {
        if (block_) {
            block_->Del();
        }
        block_ = new PointingControlBlock<Y>(ptr);
        ptr_ = ptr;
    }
    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
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
        if (!block_) {
            return 0;
        }
        return block_->GetCnt();
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    template <typename Y, typename... Args>
    friend SharedPtr<Y> MakeShared(Args&&... args);

    template <typename Y>
    friend class SharedPtr;

private:
    T* ptr_;
    ControlBlock* block_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> result;
    EmplacingControlBlock<T>* emplacing_ptr =
        new EmplacingControlBlock<T>(std::forward<Args>(args)...);
    result.block_ = emplacing_ptr;
    result.ptr_ = emplacing_ptr->Get();
    return result;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
