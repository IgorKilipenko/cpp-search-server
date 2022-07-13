#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>

#include "array_ptr.h"

class ReserveProxyObj {
   public:
    explicit ReserveProxyObj(size_t size) : capacity_{size} {}
    size_t GetSize() const {
        return capacity_;
    }

   private:
    size_t capacity_ = 0;
};

template <typename Type>
class SimpleVector {
   public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept;

    /// Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size);

    explicit SimpleVector(ReserveProxyObj capacity);

    /// Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value);

    SimpleVector(ConstIterator begin, ConstIterator end);

    /// Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init);

    SimpleVector(const SimpleVector& other);

    SimpleVector& operator=(const SimpleVector& rhs);

    /// Возвращает количество элементов в массиве
    size_t GetSize() const noexcept;

    /// Возвращает вместимость массива
    size_t GetCapacity() const noexcept;

    /// Сообщает, пустой ли массив
    bool IsEmpty() const noexcept;

    /// Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept;

    /// Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept;

    /// Возвращает константную ссылку на элемент с индексом index
    /// Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index);

    /// Возвращает константную ссылку на элемент с индексом index
    /// Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const;

    /// Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept;

    /// Изменяет размер массива.
    /// При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size);

    /// Возвращает итератор на начало массива
    /// Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept;

    /// Возвращает итератор на элемент, следующий за последним
    /// Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept;

    /// Возвращает константный итератор на начало массива
    /// Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept;

    /// Возвращает итератор на элемент, следующий за последним
    /// Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept;

    /// Возвращает константный итератор на начало массива
    /// Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept;

    /// Возвращает итератор на элемент, следующий за последним
    /// Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept;

    /// Добавляет элемент в конец вектора
    /// При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item);

    /// Вставляет значение value в позицию pos.
    /// Возвращает итератор на вставленное значение
    /// Если перед вставкой значения вектор был заполнен полностью,
    /// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value);

    /// "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept;

    /// Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos);

    /// Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept;

    void Reserve(size_t new_capacity);

   private:
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

// ----------------------------------------------------------------
// SimpleVector operators overrides and helper functions
// ----------------------------------------------------------------

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

inline ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

// ----------------------------------------------------------------
// SimpleVector implementation
// ----------------------------------------------------------------

template <typename Type>
SimpleVector<Type>::SimpleVector() noexcept : SimpleVector(0) {}

template <typename Type>
SimpleVector<Type>::SimpleVector(size_t size) : array_(size, Type()), size_{size}, capacity_{size} {}

template <typename Type>
SimpleVector<Type>::SimpleVector(ReserveProxyObj capacity) : SimpleVector() {
    Reserve(capacity.GetSize());
}

template <typename Type>
SimpleVector<Type>::SimpleVector(size_t size, const Type& value) : array_(size, value), size_{size}, capacity_{size} {}

template <typename Type>
SimpleVector<Type>::SimpleVector(ConstIterator begin, ConstIterator end) : SimpleVector((assert(end >= begin), end - begin)) {
    if (begin == end) {
        return;
    }
    std::copy(begin, end, this->begin());
}

template <typename Type>
SimpleVector<Type>::SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.begin(), init.end()) {}

template <typename Type>
SimpleVector<Type>::SimpleVector(const SimpleVector& other) {
    *this = other;
}

template <typename Type>
size_t SimpleVector<Type>::GetSize() const noexcept {
    return size_;
}

template <typename Type>
size_t SimpleVector<Type>::GetCapacity() const noexcept {
    return capacity_;
}

template <typename Type>
bool SimpleVector<Type>::IsEmpty() const noexcept {
    return size_ == 0;
}

template <typename Type>
Type& SimpleVector<Type>::operator[](size_t index) noexcept {
    return begin()[index];
}

template <typename Type>
const Type& SimpleVector<Type>::operator[](size_t index) const noexcept {
    return begin()[index];
}

template <typename Type>
SimpleVector<Type>& SimpleVector<Type>::operator=(const SimpleVector& rhs) {
    if (*this == rhs) {
        return *this;
    }
    if (rhs.GetCapacity() == 0) {
        Clear();
    }
    SimpleVector tmp(rhs.cbegin(), rhs.cend());
    this->swap(tmp);

    return *this;
}

template <typename Type>
Type& SimpleVector<Type>::At(size_t index) {
    if (index >= GetSize()) {
        throw std::out_of_range("index out of range");
    }
    return array_[index];
}

template <typename Type>
const Type& SimpleVector<Type>::At(size_t index) const {
    if (index >= GetSize()) {
        throw std::out_of_range("index out of range");
    }
    return array_[index];
}

template <typename Type>
void SimpleVector<Type>::Clear() noexcept {
    Resize(0);
}

template <typename Type>
void SimpleVector<Type>::Resize(size_t new_size) {
    if (new_size > size_ && new_size <= capacity_) {
        size_t old_size = size_;
        size_ = new_size;
        std::fill(begin() + old_size, begin() + new_size, Type());
    } else if (new_size > capacity_) {
        SimpleVector buffer{begin(), end()};
        buffer.Reserve(new_size);
        buffer.size_ = new_size;
        std::fill(buffer.begin() + size_, buffer.end(), Type());
        this->swap(buffer);
    } else {
        size_ = new_size;
    }
}

template <typename Type>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::begin() noexcept {
    return size_ ? &(array_[0]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::end() noexcept {
    return size_ ? &(array_[size_]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::begin() const noexcept {
    return size_ ? &(array_[0]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::end() const noexcept {
    return size_ ? &(array_[size_]) : nullptr;
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::cbegin() const noexcept {
    return begin();
}

template <typename Type>
typename SimpleVector<Type>::ConstIterator SimpleVector<Type>::cend() const noexcept {
    return end();
}

template <typename Type>
void SimpleVector<Type>::PushBack(const Type& item) {
    size_t old_size = size_;
    if (size_ == capacity_) {
        size_t new_capacity = std::max(capacity_, 1ul) * 2;
        Reserve(new_capacity);
    }
    Resize(old_size + 1);
    array_[old_size] = item;
}

template <typename Type>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::Insert(ConstIterator pos, const Type& value) {
    if (pos == end() && capacity_ > size_) {
        array_[size_++] = value;
        return end() - 1;
    }
    size_t new_capacity = size_ == capacity_ ? std::max(capacity_, 1ul) * 2 : capacity_;
    size_t new_size = size_ + 1;
    size_t input_index = pos - cbegin();
    SimpleVector buffer{cbegin(), cend()};
    buffer.Reserve(new_capacity);
    buffer.Resize(new_size);

    std::copy_backward(pos, cend(), buffer.end());

    buffer[input_index] = value;
    this->swap(buffer);

    return begin() + input_index;
}

template <typename Type>
void SimpleVector<Type>::PopBack() noexcept {
    --size_;
}

template <typename Type>
typename SimpleVector<Type>::Iterator SimpleVector<Type>::Erase(ConstIterator pos) {
    size_t erase_index = pos - cbegin();
    Iterator new_first = &(array_[erase_index]);
    std::copy(pos + 1, cend(), new_first);
    --size_;
    return begin() + erase_index;
}

template <typename Type>
void SimpleVector<Type>::swap(SimpleVector& other) noexcept {
    if (this == &other) {
        return;
    }
    this->array_.swap(other.array_);
    std::swap(this->size_, other.size_);
    std::swap(this->capacity_, other.capacity_);
}

template <typename Type>
void SimpleVector<Type>::Reserve(size_t new_capacity) {
    if (new_capacity <= capacity_) {
        return;
    }
    SimpleVector tmp(new_capacity);
    tmp.size_ = size_;
    std::copy(begin(), end(), tmp.begin());
    this->swap(tmp);
}
