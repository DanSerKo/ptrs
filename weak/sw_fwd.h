#pragma once

#include <exception>
#include <cstddef>

class ControlBlock {
public:
    ControlBlock() {
        shared_cnt_ = 1;
        weak_cnt_ = 1;
    }
    virtual ~ControlBlock() = default;

    void AddShared() {
        shared_cnt_++;
        AddWeak();
    }
    void DelShared() {
        shared_cnt_--;
        if (shared_cnt_ == 0) {
            OnZeroShared();
        }
        DelWeak();
    }

    void AddWeak() {
        weak_cnt_++;
    }
    void DelWeak() {
        weak_cnt_--;
        if (weak_cnt_ == 0) {
            OnZeroWeak();
        }
    }

    virtual void OnZeroShared() = 0;
    virtual void OnZeroWeak() = 0;

    size_t GetCnt() const {
        return shared_cnt_;
    }

private:
    size_t shared_cnt_;
    size_t weak_cnt_;
};

template <typename T>
class PointingControlBlock : public ControlBlock {
public:
    PointingControlBlock(T* ptr) : ControlBlock(), ptr_(ptr) {
    }
    virtual ~PointingControlBlock() = default;
    void OnZeroShared() override {
        if (ptr_) {
            delete ptr_;
        }
    }
    void OnZeroWeak() override {
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

    void OnZeroShared() override {
        Get()->~T();
    }
    void OnZeroWeak() override {
        delete this;
    }

    T* Get() {
        return reinterpret_cast<T*>(&buffer_);
    }

private:
    alignas(T) std::byte buffer_[sizeof(T)];
};

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
