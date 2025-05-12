#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <utility>

template <typename T>
class DefaultDeleter {
public:
    DefaultDeleter() {
    }
    template <typename F>
    DefaultDeleter(DefaultDeleter<F>) {
    }
    void operator()(T* p) const {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete p;
    }
};

template <typename T>
class DefaultDeleter<T[]> {
public:
    DefaultDeleter() {
    }
    template <typename F>
    DefaultDeleter(DefaultDeleter<F>) {
    }
    void operator()(T* p) const {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete[] p;
    }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        data_ = std::move(other.data_);
        other.data_ = CompressedPair<T*, Deleter>(nullptr, Deleter());
    }

    template <typename T2, typename Deleter2>
    UniquePtr(UniquePtr<T2, Deleter2>&& other) noexcept
        : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {  // swap
        if (this == &other) {
            return *this;
        }
        Reset();
        data_ = std::move(other.data_);
        other.data_ = CompressedPair<T*, Deleter>(nullptr, Deleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ~UniquePtr() {
        Reset();
    }

    T* Release() {
        T* tmp = data_.GetFirst();
        data_ = CompressedPair<T*, Deleter>(nullptr, Deleter());
        return tmp;
    }
    void Reset(T* ptr = nullptr) {
        T* old = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (old) {
            GetDeleter()(old);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst() != nullptr;
    }

    std::add_lvalue_reference_t<T> operator*() const {
        return *data_.GetFirst();
    }
    T* operator->() const {
        return data_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> data_;
};

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        data_ = std::move(other.data_);
        other.data_ = CompressedPair<T*, Deleter>(nullptr, Deleter());
    }

    template <typename T2, typename Deleter2>
    UniquePtr(UniquePtr<T2, Deleter2>&& other) noexcept
        : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {  // swap
        if (this == &other) {
            return *this;
        }
        Reset();
        data_ = std::move(other.data_);
        other.data_ = CompressedPair<T*, Deleter>(nullptr, Deleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ~UniquePtr() {
        Reset();
    }

    T* Release() {
        T* tmp = data_.GetFirst();
        data_ = CompressedPair<T*, Deleter>(nullptr, Deleter());
        return tmp;
    }
    void Reset(T* ptr = nullptr) {
        T* old = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (old) {
            GetDeleter()(old);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst() != nullptr;
    }

    T& operator[](size_t index) {
        return data_.GetFirst()[index];
    }
    const T& operator[](size_t index) const {
        return data_.GetFirst()[index];
    }

private:
    CompressedPair<T*, Deleter> data_;
};
