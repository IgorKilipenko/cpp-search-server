#include <algorithm>
#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <vector>

#include "log_duration.h"

using namespace std;

template <typename It>
void PrintRange(It range_begin, It range_end) {
    for (auto it = range_begin; it != range_end; ++it) {
        cout << *it << " "s;
    }
    cout << endl;
}

// Ускорьте с помощью параллельности
template <typename RandomIt>
void MergeSortSync(RandomIt range_begin, RandomIt range_end) {
    // 1. Если диапазон содержит меньше 2 элементов, выходим из функции
    int range_length = range_end - range_begin;
    if (range_length < 2) {
        return;
    }

    // 2. Создаём вектор, содержащий все элементы текущего диапазона
    vector elements(range_begin, range_end);
    // Тип элементов — typename iterator_traits<RandomIt>::value_type

    // 3. Разбиваем вектор на две равные части
    auto mid = elements.begin() + range_length / 2;

    // 4. Вызываем функцию MergeSort от каждой половины вектора
    MergeSortSync(elements.begin(), mid);
    MergeSortSync(mid, elements.end());

    // 5. С помощью алгоритма merge сливаем отсортированные половины
    // в исходный диапазон
    // merge -> http://ru.cppreference.com/w/cpp/algorithm/merge
    merge(elements.begin(), mid, mid, elements.end(), range_begin);
}

// Ускорьте с помощью параллельности
template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    vector elements(make_move_iterator(range_begin), make_move_iterator(range_end));
    auto begin = elements.begin();
    auto end = elements.end();
    auto mid = begin + elements.size() / 2;
    auto f1 = async([&]() {
        MergeSortSync(begin, mid);
    });
    auto f2 = async([&]() {
        MergeSortSync(mid, end);
    });
    f1.get();
    f2.get();
    merge(begin, mid, mid, end, range_begin);
}

int main() {
    mt19937 generator;

    vector<int> test_vector(4'000'000);

    // iota             -> http://ru.cppreference.com/w/cpp/algorithm/iota
    // Заполняет диапазон последовательно возрастающими значениями
    iota(test_vector.begin(), test_vector.end(), 1);

    // shuffle   -> https://ru.cppreference.com/w/cpp/algorithm/random_shuffle
    // Перемешивает элементы в случайном порядке
    // shuffle(test_vector.begin(), test_vector.end(), generator);
    /*
        // Выводим вектор до сортировки
        PrintRange(test_vector.begin(), test_vector.end());

        // Сортируем вектор с помощью сортировки слиянием
        MergeSort(test_vector.begin(), test_vector.end());

        // Выводим результат
        PrintRange(test_vector.begin(), test_vector.end());
    */
    {
        LOG_DURATION_STREAM("MergeSort", cerr);
        // Проверяем, можно ли передать указатели
        MergeSortSync(test_vector.data(), test_vector.data() + test_vector.size());
    }
    {
        LOG_DURATION_STREAM("MergeSort async", cerr);
        // Проверяем, можно ли передать указатели
        MergeSort(test_vector.data(), test_vector.data() + test_vector.size());
    }

    return 0;
}