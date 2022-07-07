#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <new>
#include <stdexcept>
#include <vector>

using namespace std;

// Умный указатель, удаляющий связанный объект при своём разрушении.
// Параметр шаблона T задаёт тип объекта, на который ссылается указатель
template <typename T>
class ScopedPtr {
   public:
    // Конструктор по умолчанию создаёт нулевой указатель,
    // так как поле ptr_ имеет значение по умолчанию nullptr
    ScopedPtr() = default;

    // Создаёт указатель, ссылающийся на переданный raw_ptr.
    // raw_ptr ссылается либо на объект, созданный в куче при помощи new,
    // либо является нулевым указателем
    // Спецификатор noexcept обозначает, что метод не бросает исключений
    explicit ScopedPtr(T* raw_ptr) noexcept {
        // Реализуйте самостоятельно
        ptr_ = raw_ptr;
    }

    // Удаляем у класса конструктор копирования
    ScopedPtr(const ScopedPtr&) = delete;

    // Деструктор. Удаляет объект, на который ссылается умный указатель.
    ~ScopedPtr() {
        // Реализуйте тело деструктора самостоятельно
        delete ptr_;
        ptr_ = nullptr;
    }

    // Возвращает указатель, хранящийся внутри ScopedPtr
    T* GetRawPtr() const noexcept {
        // Напишите код метода самостоятельно
        return ptr_;
    }

    // Прекращает владение объектом, на который ссылается умный указатель.
    // Возвращает прежнее значение "сырого" указателя и устанавливает поле ptr_ в null
    T* Release() noexcept {
        // Реализуйте самостоятельно
        T* result = ptr_;
        ptr_ = nullptr;
        return result;
    }

    T& operator*() {
        if (ptr_ == nullptr) {
            throw std::logic_error("Is null pointer.");
        }
        return *ptr_;
    }

    T* operator->() {
        if (ptr_ == nullptr) {
            throw std::logic_error("Is null pointer.");
        }
        return ptr_;
    }

    operator bool() const {
        return ptr_ != nullptr;
    }

   private:
    T* ptr_ = nullptr;
};

template <typename T>
class PtrVector {
   public:
    PtrVector() = default;

    explicit PtrVector(size_t size) : items_(size, nullptr) {}

    // Создаёт вектор указателей на копии объектов из other
    PtrVector(const PtrVector& other) {
        // Реализуйте копирующий конструктор самостоятельно
        if (other.items_.empty()) {
            return;
        }
        items_.reserve(other.items_.size());
        const auto& src_items = other.items_;
        try {
            for (const T* const src_item_ptr : src_items) {
                T* ptr = src_item_ptr != nullptr ? new T(*src_item_ptr) : nullptr;
                items_.push_back(ptr);
            }
        } catch (...) {
            FreeItems();
            throw;
        }
    }

    PtrVector& operator=(const PtrVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        try {
            auto rhs_copy(rhs);
            swap(rhs_copy);
        } catch (...) {
            throw;
        }
        return *this;
    }

    // Деструктор удаляет объекты в куче, на которые ссылаются указатели,
    // в векторе items_
    ~PtrVector() {
        // Реализуйте тело деструктора самостоятельно
        FreeItems();
    }

    // Возвращает ссылку на вектор указателей
    vector<T*>& GetItems() noexcept {
        // Реализуйте метод самостоятельно
        return items_;
    }

    // Возвращает константную ссылку на вектор указателей
    vector<T*> const& GetItems() const noexcept {
        // Реализуйте метод самостоятельно
        return items_;
    }

   private:
    vector<T*> items_;
    void FreeItems() {
        for (T* item : items_) {
            delete item;
        }
        items_.clear();
    }
    void swap(PtrVector& other) noexcept {
        items_.swap(other.items_);
    }
};

class Tentacle {
   public:
    Tentacle(size_t id, Tentacle* link = nullptr) : id_{id}, link_{link} {}
    int GetId() const {
        return id_;
    }
    Tentacle* GetLinkedTentacle() const {
        return link_;
    }
    void LinkTo(Tentacle& tentacle) {
        link_ = &tentacle;
    }
    void Unlink() {
        link_ = nullptr;
    }

   private:
    size_t id_ = 0;
    Tentacle* link_;
};

class Octopus {
   public:
    explicit Octopus(size_t tentacle_count = 8) : tentaclesies_(tentacle_count) {
        auto& items = tentaclesies_.GetItems();
        for (int i = 0; i < static_cast<int>(tentacle_count); i++) {
            items[i] = new Tentacle(i + 1);
        }
    }
    int GetTentacleCount() const {
        return tentaclesies_.GetItems().size();
    }
    Tentacle& GetTentacle(int index) const {
        return *tentaclesies_.GetItems().at(index);
    }
    Tentacle& AddTentacle() {
        Tentacle* new_tentacle = new Tentacle(GetTentacleCount() + 1);
        tentaclesies_.GetItems().push_back(new_tentacle);
        return *new_tentacle;
    }

   private:
    PtrVector<Tentacle> tentaclesies_;
};

int main() {
    // Проверка присваивания осьминогов
    {
        Octopus octopus1(3);

        // Настраиваем состояние исходного осьминога
        octopus1.GetTentacle(2).LinkTo(octopus1.GetTentacle(1));

        // До присваивания octopus2 имеет своё собственное состояние
        Octopus octopus2(10);

        octopus2 = octopus1;

        // После присваивания осьминогов щупальца копии имеют то же состояние,
        // что и щупальца присваиваемого объекта
        assert(octopus2.GetTentacleCount() == octopus1.GetTentacleCount());
        for (int i = 0; i < octopus2.GetTentacleCount(); ++i) {
            auto& tentacle1 = octopus1.GetTentacle(i);
            auto& tentacle2 = octopus2.GetTentacle(i);
            assert(&tentacle2 != &tentacle1);
            assert(tentacle2.GetId() == tentacle1.GetId());
            assert(tentacle2.GetLinkedTentacle() == tentacle1.GetLinkedTentacle());
        }
    }

    // Проверка самоприсваивания осьминогов
    {
        Octopus octopus(3);

        // Настраиваем состояние осьминога
        octopus.GetTentacle(0).LinkTo(octopus.GetTentacle(1));

        vector<pair<Tentacle*, Tentacle*>> tentacles;
        // Сохраняем информацию о щупальцах осьминога и его копии
        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            tentacles.push_back({&octopus.GetTentacle(i), octopus.GetTentacle(i).GetLinkedTentacle()});
        }

        // Выполняем самоприсваивание
        //// octopus = octopus;

        // После самоприсваивания состояние осьминога не должно измениться
        assert(octopus.GetTentacleCount() == static_cast<int>(tentacles.size()));
        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            auto& tentacle_with_link = tentacles.at(i);
            assert(&octopus.GetTentacle(i) == tentacle_with_link.first);
            assert(octopus.GetTentacle(i).GetLinkedTentacle() == tentacle_with_link.second);
        }
    }
} 
