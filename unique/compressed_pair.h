#pragma once

#include <type_traits>
#include <utility>

template <typename T, bool IsEmpty = std::is_empty<T>::value && !std::is_final<T>::value>
class First;
template <typename T, bool IsEmpty = std::is_empty<T>::value && !std::is_final<T>::value>
class Second;

template <typename T>
class First<T, true> : public T {
public:
    First() : T() {
    }
    First(const T& value) : T(value) {
    }
    First(T&& value) : T(std::move(value)) {
    }
    T& GetFirst() {
        return static_cast<T&>(*this);
    }
    const T& GetFirst() const {
        return static_cast<const T&>(*this);
    }

private:
};

template <typename T>
class First<T, false> {
public:
    First() : value_() {
    }
    First(const T& value) : value_(value) {
    }
    First(T&& value) : value_(std::move(value)) {
    }
    T& GetFirst() {
        return value_;
    }
    const T& GetFirst() const {
        return value_;
    }

private:
    T value_;
};

template <typename T>
class Second<T, true> : public T {
public:
    Second() : T() {
    }
    Second(const T& value) : T(value) {
    }
    Second(T&& value) : T(std::move(value)) {
    }
    T& GetSecond() {
        return static_cast<T&>(*this);
    }
    const T& GetSecond() const {
        return static_cast<const T&>(*this);
    }

private:
};

template <typename T>
class Second<T, false> {
public:
    Second() : value_() {
    }
    Second(const T& value) : value_(value) {
    }
    Second(T&& value) : value_(std::move(value)) {
    }
    T& GetSecond() {
        return value_;
    }
    const T& GetSecond() const {
        return value_;
    }

private:
    T value_;
};

template <typename F, typename S>
class CompressedPair : public First<F>, public Second<S> {
public:
    CompressedPair() : First<F>(), Second<S>() {
    }
    CompressedPair(const F& first, const S& second) : First<F>(first), Second<S>(second) {
    }
    CompressedPair(F&& first, const S& second) : First<F>(std::move(first)), Second<S>(second) {
    }
    CompressedPair(const F& first, S&& second) : First<F>(first), Second<S>(std::move(second)) {
    }

    CompressedPair(F&& first, S&& second)
        : First<F>(std::move(first)), Second<S>(std::move(second)) {
    }

private:
};
