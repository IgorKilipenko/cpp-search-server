#pragma once

#include <array>
#include <cassert>
#include <iterator>
#include <stdexcept>

template <typename T, size_t N>
class StackVector {
   public:
    using Iterator = typename std::array<T, N>::iterator;
    using ConstIterator = typename std::array<T, N>::const_iterator;

    explicit StackVector(size_t a_size = 0) : cursor_{static_cast<int>(a_size) - 1} {
        size_t size = instance_.size();
        if (static_cast<size_t>(cursor_ + 1) > size) {
            cursor_ = size - 1;
            throw std::invalid_argument("Buffer overflow.");
        }
    }

    T& operator[](size_t index) {
        assert(index < Size());

        return instance_[index];
    }
    const T& operator[](size_t index) const {
        assert(index < Size());

        return instance_[index];
    }

    Iterator begin() {
        return instance_.begin();
    }
    Iterator end() {
        return instance_.begin() + cursor_ + 1;
    }
    ConstIterator begin() const {
        return instance_.begin();
    }
    ConstIterator end() const {
        return instance_.begin() + cursor_ + 1;
    }

    size_t Size() const {
        if (instance_.empty()) {
            return 0;
        }
        return cursor_ + 1;
    }
    size_t Capacity() const {
        return instance_.size();
    }

    void PushBack(const T& value) {
        if (static_cast<size_t>(cursor_ + 1) >= instance_.size()) {
            throw std::overflow_error("Index out of range.");
        }

        instance_[++cursor_] = value;
    }
    T PopBack() {
        if (!Size()) {
            throw std::underflow_error("Underflow error.");
        }

        T result = instance_[cursor_];
        --cursor_;
        return result;
    }

   private:
    std::array<T, N> instance_;
    int cursor_;
};
