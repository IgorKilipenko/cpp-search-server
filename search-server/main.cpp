#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <string_view>

#include "log_duration.h"

using namespace std;

string GenerateQuery(mt19937& generator, int max_length, int space_rate) {
    const int length = uniform_int_distribution(0, max_length)(generator);
    string query(length, ' ');
    for (char& c : query) {
        const int rnd = uniform_int_distribution(0, space_rate - 1)(generator);
        if (rnd > 0) {
            c = 'a' + (rnd - 1);
        }
    }
    return query;
}

template <typename Solver>
void Test(string_view mark, string_view s, Solver solver) {
    int result;
    {
        LOG_DURATION(mark);
        result = solver(s);
    }
    cout << result << endl;
}

#define TEST(solver) Test(#solver, s, solver)

int CountWords(string_view str) {
    // подсчитайте количество слов,
    // игнорируя начальные, конечные
    // и подряд идущие пробелы

    // некорректное решение, с которым сравнивается производительность
    if (str.empty()) {
        return 0;
    }
    auto ptr = std::find_if(str.begin(), str.end(), [](const char c) {
        return c != ' ';
    });
    if (ptr == str.end()) {
        return 0;
    }
    size_t size = transform_reduce(ptr, prev(str.end()), next(ptr), 0ul, std::plus<>{},
                                   [](const char cur, const char next) {
                                       return cur != ' ' && next == ' ';
                                   }) +
                  (*prev(str.end()) != ' ');
    return static_cast<int>(size);
}

int main() {
    // должно вывести 3
    cout << CountWords("  pretty  little octopus "sv) << endl;

    mt19937 generator;

    const string s = GenerateQuery(generator, 100'000'000, 4);

    TEST(CountWords);

    return 0;
}