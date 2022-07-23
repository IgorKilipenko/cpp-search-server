#include <algorithm>
#include <cstddef>
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
    thread_count = size / items_per_thread;
    if (thread_count < 2) {
        MergeSortSync(range_begin, range_end);
    }
    vector<std::future<vector<typename iterator_traits<RandomIt>::value_type>>> actions{thread_count};
    for (size_t i = 0; i < thread_count; ++i) {
        auto begin = range_begin + (i * items_per_thread);
        auto end = (range_end - begin > items_per_thread) ? begin + items_per_thread : range_end;
        
        actions[i] = async([begin, end]() {
            vector<typename iterator_traits<RandomIt>::value_type> items(begin, end);
            auto mid = items.begin() + items.size() / 2;
            vector<typename iterator_traits<RandomIt>::value_type> result(items.size());
            MergeSortSync(items.begin(), mid);
            MergeSortSync(mid, items.end());
            merge(items.begin(), mid, mid, items.end(), result.begin());
            return result;
        });
    }

    vector<vector<typename iterator_traits<RandomIt>::value_type>> parts;
    for (size_t i = 1; i < actions.size(); i += 2) {
        auto part1 = actions[i - 1].get();
        auto part2 = actions[i].get();
        vector<typename iterator_traits<RandomIt>::value_type> result(part1.size() + part2.size());
        merge(part1.begin(), part1.end(), part2.begin(), part2.end(), result.begin());
        parts.push_back(result);
    }

    for (; parts.size() > 1;) {
        auto part1 = parts.back();
        parts.pop_back();
        auto part2 = parts.back();
        parts.pop_back();
        vector<typename iterator_traits<RandomIt>::value_type> result(part1.size() + part2.size());
        merge(part1.begin(), part1.end(), part2.begin(), part2.end(), result.begin());
        parts.push_back(result);
    }

    if (!parts.empty()) {
        std::move(parts.back().begin(), parts.back().end(), range_begin);
    }

    /*auto begin = elements.begin();
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
    merge(begin, mid, mid, end, range_begin);*/
}

int main() {
    mt19937 generator;

    vector<int> test_vector(7);

    // iota             -> http://ru.cppreference.com/w/cpp/algorithm/iota
    // Заполняет диапазон последовательно возрастающими значениями
    iota(test_vector.begin(), test_vector.end(), 1);

    // shuffle   -> https://ru.cppreference.com/w/cpp/algorithm/random_shuffle
    // Перемешивает элементы в случайном порядке
    shuffle(test_vector.begin(), test_vector.end(), generator);
    /*
        // Выводим вектор до сортировки
        PrintRange(test_vector.begin(), test_vector.end());

        // Сортируем вектор с помощью сортировки слиянием
        MergeSort(test_vector.begin(), test_vector.end());

        // Выводим результат
        PrintRange(test_vector.begin(), test_vector.end());
    */
    {
        //LOG_DURATION_STREAM("MergeSort", cerr);
        // Проверяем, можно ли передать указатели
        //MergeSortSync(test_vector.data(), test_vector.data() + test_vector.size());
    }
    {
        LOG_DURATION_STREAM("MergeSort async", cerr);
        // Проверяем, можно ли передать указатели
        MergeSort(test_vector.data(), test_vector.data() + test_vector.size());
        PrintRange(test_vector.begin(), test_vector.end());
    }

    return 0;
}