#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <type_traits>
#include <initializer_list>


// Remove this sheet when update to C++26

template <typename T, std::size_t Capacity>
class inplace_vector {
public:
    inplace_vector() noexcept : size_(0) {}
    
    inplace_vector(const std::initializer_list<T> l) : inplace_vector() {
        for (auto& x: l)
            push_back(x);
    }

    inplace_vector(const inplace_vector& another) : inplace_vector() {
        for (const auto& x: another)
            push_back(x);
    }

    inplace_vector(inplace_vector&& another) : inplace_vector() {
        for (auto& x: another)
            push_back(std::move(x));
        another.clear();
    }

    ~inplace_vector() {
        clear();
    }

    std::size_t size() const noexcept {
        return size_;
    }

    constexpr std::size_t capacity() const noexcept {
        return Capacity;
    }

    void clear() noexcept {
        for (std::size_t i = 0; i < size_; ++i) {
            data()[i].~T();
        }
        size_ = 0;
    }

    bool resize(std::size_t new_size) {
        if (new_size > Capacity) {
            return false;
        }

        if (new_size > size_) {
            for (std::size_t i = size_; i < new_size; ++i) {
                new (data() + i) T();
            }
        } else {
            for (std::size_t i = new_size; i < size_; ++i) {
                data()[i].~T();
            }
        }

        size_ = new_size;
        return true;
    }

    template <typename... Args>
    T* emplace_back(Args&&... args) {
        if (size_ >= Capacity) {
            return NULL;
        }
        new (data() + size_) T(std::forward<Args>(args)...);
        return data() + size_++;
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    T& operator[](std::size_t i) {
        return data()[i];
    }

    const T& operator[](std::size_t i) const {
        return data()[i];
    }


    T* begin() {
        return data();
    }


    T* end() {
        return data() + size_;
    }


    const T* begin() const {
        return data();
    }


    const T* end() const {
        return data() + size_;
    }


    void operator=(const inplace_vector& another) {
        clear();
        for (const auto& x: another)
            push_back(x);
    }
    

    void operator=(inplace_vector&& another) {
        clear();
        for (auto& x: another)
            push_back(std::move(x));
        another.clear();
    }

    
private:

    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer_[Capacity];

    std::size_t size_;

    T* data() noexcept {
        return reinterpret_cast<T*>(buffer_);
    }

    const T* data() const noexcept {
        return reinterpret_cast<const T*>(buffer_);
    }
};
