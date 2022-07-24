#include <pstl/glue_execution_defs.h>

#include <algorithm>
#include <cstddef>
#include <execution>
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
    size_t size = range_end - range_begin;
    size_t thread_count = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4ul;
    int items_per_thread = max(static_cast<size_t>(size * 2 / thread_count), 2ul);
    items_per_thread = (size % items_per_thread > 0 && size % items_per_thread < 2) ? items_per_thread + 1 : items_per_thread;
    thread_count = size % items_per_thread ? size / items_per_thread + 1 : size / items_per_thread;
    if (thread_count < 2) {
        MergeSortSync(range_begin, range_end);
    }
    vector<std::future<vector<typename iterator_traits<RandomIt>::value_type>>> actions{thread_count};
    for (size_t i = 0; i < thread_count; ++i) {
        auto begin = range_begin + (i * items_per_thread);
        auto end = (range_end - begin > items_per_thread) ? begin + items_per_thread : range_end;
        actions[i] = async([begin, end]() {
            vector<typename iterator_traits<RandomIt>::value_type> items(make_move_iterator(begin), make_move_iterator(end));
            auto mid = items.begin() + items.size() / 2;
            vector<typename iterator_traits<RandomIt>::value_type> result(items.size());
            MergeSortSync(items.begin(), mid);
            MergeSortSync(mid, items.end());
            merge(items.begin(), mid, mid, items.end(), result.begin());
            return result;
        });
    }

    vector<typename iterator_traits<RandomIt>::value_type> result = actions[0].get();
    for (size_t i = 1; i < actions.size(); ++i) {
        auto part = actions[i].get();
        vector<typename iterator_traits<RandomIt>::value_type> part_merged(part.size() + result.size());
        merge(result.begin(), result.end(), part.begin(), part.end(), part_merged.begin());
        result.swap(part_merged);
    }

    std::move(result.begin(), result.end(), range_begin);
}

int main() {
    mt19937 generator;

    vector<int> test_vector(4'000'000);

    // iota             -> http://ru.cppreference.com/w/cpp/algorithm/iota
    // Заполняет диапазон последовательно возрастающими значениями
    iota(test_vector.begin(), test_vector.end(), 1);

    // shuffle   -> https://ru.cppreference.com/w/cpp/algorithm/random_shuffle
    // Перемешивает элементы в случайном порядке
    shuffle(test_vector.begin(), test_vector.end(), generator);

    // Выводим вектор до сортировки
    // PrintRange(test_vector.begin(), test_vector.end());

    // Сортируем вектор с помощью сортировки слиянием
    // MergeSort(test_vector.begin(), test_vector.end());

    // Выводим результат
    // PrintRange(test_vector.begin(), test_vector.end());

    {
        vector data(test_vector.begin(), test_vector.end());
        LOG_DURATION_STREAM("MergeSort", cerr);
        //  Проверяем, можно ли передать указатели
        MergeSortSync(data.data(), data.data() + data.size());
    }
    {
        vector data(test_vector.begin(), test_vector.end());
        LOG_DURATION_STREAM("MergeSort async", cerr);
        // Проверяем, можно ли передать указатели
        MergeSort(data.data(), data.data() + data.size());
    }

    return 0;
}