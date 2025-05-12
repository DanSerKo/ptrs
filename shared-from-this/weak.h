#pragma once

#include "sw_fwd.h"

template <typename T>
class WeakPtr {
public:
    WeakPtr() {
        block_ = nullptr;
        ptr_ = nullptr;
    }

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->AddWeak();
        }
    }
    WeakPtr(WeakPtr&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->AddWeak();
        }
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->AddWeak();
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
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

    ~WeakPtr() {
        Reset();
    }

    void Reset() {
        if (block_) {
            block_->DelWeak();
        }
        block_ = nullptr;
        ptr_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->GetCnt();
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const {
        SharedPtr<T> result;
        if (Expired()) {
            result.block_ = nullptr;
            result.ptr_ = nullptr;
            return result;
        }
        result.block_ = block_;
        result.ptr_ = ptr_;
        if (block_) {
            block_->AddShared();
        }
        return result;
    }

private:
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;

    T* ptr_;
    ControlBlock* block_;
};
