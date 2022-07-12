#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>

#include "array_ptr.h"

template <typename Type>
class SimpleVector {
   public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept : SimpleVector(0) {}

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : array_(size, Type()), size_{size}, capacity_{size} {}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : array_(size, value), size_{size}, capacity_{size} {}

    SimpleVector(ConstIterator begin, ConstIterator end) : SimpleVector((assert(end >= begin), end - begin)) {
        if (begin == end) {
            return;
        }
        std::copy(begin, end, this->begin());
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.begin(), init.end()) {}

    SimpleVector(const SimpleVector& other) {
        *this = other;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return begin()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return begin()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= GetSize()) {
            throw std::out_of_range("index out of range");
        }
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= GetSize()) {
            throw std::out_of_range("index out of range");
        }
        return array_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        Resize(0);
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        Resize(new_size, true);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return size_ ? &(array_[0]) : nullptr;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return size_ ? &(array_[size_]) : nullptr;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return size_ ? &(array_[0]) : nullptr;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return size_ ? &(array_[size_]) : nullptr;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        if (rhs.GetCapacity() == 0) {
            Clear();
        }
        //try {
            SimpleVector tmp(rhs.cbegin(), rhs.cend());
            this->swap(tmp);
        /*} catch (...) {
        }*/
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        size_t old_size = size_;
        if (size_ == capacity_) {
            size_t new_capacity = std::max(capacity_, 1ul) * 2;
            Resize(new_capacity, false);
        }
        Resize(old_size + 1);
        array_[old_size] = item;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        if (pos == end() && capacity_ > size_) {
            array_[size_++] = value;
            return end() - 1;
        }
        size_t new_capacity = size_ == capacity_ ? std::max(capacity_, 1ul) * 2 : capacity_;
        size_t new_size = size_ + 1;
        size_t input_index = pos - cbegin();
        ArrayPtr<Type> buffer{capacity_};

        Iterator first = &(buffer[0]);
        Iterator last = &(buffer[new_size]);
        std::copy(cbegin(), pos, first);
        std::copy_backward(pos, cend(), last);

        buffer[input_index] = value;
        array_.swap(buffer);

        size_ = new_size;
        capacity_ = new_capacity;

        return &(array_[input_index]);
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t erase_index = pos - cbegin();
        Iterator new_first = &(array_[erase_index]);
        std::copy(pos + 1, cend(), new_first);
        --size_;
        return &(array_[erase_index]);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        if (this == &other) {
            return;
        }
        this->array_.swap(other.array_);
        std::swap(this->size_, other.size_);
        std::swap(this->capacity_, other.capacity_);
    }

   private:
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void Resize(size_t new_size, bool initial_all) {
        if (new_size > size_ && new_size <= capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                array_[i] = Type();
            }
        }
        if (new_size > capacity_) {
            Type* buffer = new Type[new_size];
            capacity_ = new_size;
            Type* old_array = array_.Get();
            for (size_t i = 0; i < size_; ++i) {
                buffer[i] = old_array[i];
            }
            if (initial_all) {
                for (size_t i = size_; i < capacity_; ++i) {
                    buffer[i] = Type();
                }
            }
            ArrayPtr<Type> new_array(buffer);
            array_.swap(new_array);
        }
        size_ = new_size;
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return &lhs == &rhs || (lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
