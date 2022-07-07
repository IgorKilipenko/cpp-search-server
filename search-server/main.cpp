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
        items_.reserve(other.items_.size());
        const auto& src_items = other.items_;
        T* ptr = nullptr;
        try {
            for (const T* const src_item_ptr : src_items) {
                ptr = src_item_ptr != nullptr ? new T(*src_item_ptr) : nullptr;
                items_.push_back(ptr);
            }
        } catch (const std::bad_alloc& ex) {
            delete ptr;
            FreeItems();
            throw std::bad_alloc();
        }
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
};

class Tentacle {
   public:
    Tentacle(size_t id, Tentacle* link = nullptr) : id_{id}, link_{link} {}
    int GetId() const {
        return id_;
    }
    const Tentacle* GetLinkedTentacle() const {
        return link_;
    }
    void LinkTo(const Tentacle& tentacle) {
        link_ = &tentacle;
    }
    void Unlink() {
        link_ = nullptr;
    }

   private:
    size_t id_ = 0;
    const Tentacle* link_;
};

class Octopus {
   public:
    explicit Octopus(size_t tentacle_count = 8) : tentaclesies_(tentacle_count) {
        auto& items = tentaclesies_.GetItems();
        for (int i = 0; i < tentacle_count; i++) {
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
    // Проверка конструирования осьминогов
    {
        // По умолчанию осьминог имеет 8 щупалец
        Octopus default_octopus;
        assert(default_octopus.GetTentacleCount() == 8);

        // Осьминог может иметь отличное от 8 количество щупалец
        Octopus quadropus(4);
        assert(quadropus.GetTentacleCount() == 4);

        // И даже вообще не иметь щупалец
        Octopus coloboque(0);
        assert(coloboque.GetTentacleCount() == 0);
    }

    // Осьминогу можно добавлять щупальца
    {
        Octopus octopus(1);
        Tentacle* t0 = &octopus.GetTentacle(0);
        Tentacle* t1 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 2);
        Tentacle* t2 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 3);

        // После добавления щупалец ранее созданные щупальца не меняют своих адресов
        assert(&octopus.GetTentacle(0) == t0);
        assert(&octopus.GetTentacle(1) == t1);
        assert(&octopus.GetTentacle(2) == t2);

        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            assert(octopus.GetTentacle(i).GetId() == i + 1);
        }
    }

    // Осьминоги могут прицепляться к щупальцам друг друга
    {
        Octopus male(2);
        Octopus female(2);

        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);

        male.GetTentacle(0).LinkTo(female.GetTentacle(1));
        assert(male.GetTentacle(0).GetLinkedTentacle() == &female.GetTentacle(1));

        male.GetTentacle(0).Unlink();
        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);
    }

    // Копия осьминога имеет свою собственную копию щупалец, которые
    // копируют состояние щупалец оригинального осьминога
    {
        // Перебираем осьминогов с разным количеством щупалец
        for (int num_tentacles = 0; num_tentacles < 10; ++num_tentacles) {
            Octopus male(num_tentacles);
            Octopus female(num_tentacles);
            // Пусть они хватают друг друга за щупальца
            for (int i = 0; i < num_tentacles; ++i) {
                male.GetTentacle(i).LinkTo(female.GetTentacle(num_tentacles - 1 - i));
            }

            Octopus male_copy(male);
            // Проверяем состояние щупалец копии
            assert(male_copy.GetTentacleCount() == male.GetTentacleCount());
            for (int i = 0; i < male_copy.GetTentacleCount(); ++i) {
                // Каждое щупальце копии размещается по адресу, отличному от адреса оригинального щупальца
                assert(&male_copy.GetTentacle(i) != &male.GetTentacle(i));
                // Каждое щупальце копии прицепляется к тому же щупальцу, что и оригинальное
                assert(male_copy.GetTentacle(i).GetLinkedTentacle() == male.GetTentacle(i).GetLinkedTentacle());
            }
        }
        // Если вы видите эту надпись, то разрушение осьминогов, скорее всего,
        // прошло без неопределённого поведения
        cout << "Everything is OK"s << endl;
    }
}