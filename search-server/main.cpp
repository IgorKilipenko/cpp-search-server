#include <pstl/glue_execution_defs.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <execution>
#include <future>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "log_duration.h"
#include "test_example_functions.h"

using namespace std;

template <typename RandomAccessIterator, typename Value>
RandomAccessIterator LowerBound(const execution::sequenced_policy&, RandomAccessIterator range_begin, RandomAccessIterator range_end,
                                const Value& value) {
    auto left_bound = range_begin;
    auto right_bound = range_end;
    while (left_bound + 1 < right_bound) {
        const auto middle = left_bound + (right_bound - left_bound) / 2;
        if (*middle < value) {
            left_bound = middle;
        } else {
            right_bound = middle;
        }
    }
    if (left_bound == range_begin && !(*left_bound < value)) {
        return left_bound;
    } else {
        return right_bound;
    }
}

template <typename RandomAccessIterator, typename Value>
RandomAccessIterator LowerBound(RandomAccessIterator range_begin, RandomAccessIterator range_end, const Value& value) {
    return LowerBound(execution::seq, range_begin, range_end, value);
}

template <typename RandomAccessIterator, typename Value>
RandomAccessIterator LowerBound(const execution::parallel_policy&, RandomAccessIterator range_begin, RandomAccessIterator range_end,
                                const Value& value) {
    if (range_end - range_begin < 5) {
        return LowerBound(std::execution::seq, range_begin, range_end, value);
    }
    auto expected = LowerBound(std::execution::seq, range_begin, range_end, value);
    [[maybe_unused]] vector<Value> tmp{range_begin, range_end};
    auto left_bound = range_begin;
    auto right_bound = range_end;
    while (left_bound + 1 < right_bound) {
        const auto left_middle = left_bound + max(static_cast<int>((right_bound - left_bound) / 3), 1);
        const auto right_middle = right_bound - max(static_cast<int>((right_bound - left_bound) / 3), 1);
        [[maybe_unused]] auto left_idx = left_middle - range_begin;
        [[maybe_unused]] auto right_idx = right_middle - range_begin;
        if (value > *right_middle) {
            left_bound = right_middle;
        } else if (value > *left_middle) {
            if (*prev(right_middle) == *left_middle) {
                left_bound = prev(right_middle);
                right_bound = right_middle;
            } else {
                left_bound = left_middle;
                right_bound = right_middle;
            }
        } else {
            right_bound = left_middle;
        }
    }
    if (left_bound == range_begin && !(*left_bound < value)) {
        if (left_bound != expected) {
            throw *expected;
        }
        return left_bound;
    } else {
        if (right_bound != expected) {
            throw *expected;
        }
        return right_bound;
    }
}

void Test() {
    size_t size = 500;
    size_t word_len = 5;
    const auto wordsGenerator = [](size_t count, size_t word_len) {
        mt19937 generator;
        vector<string> result{count};
        for (size_t i = 0; i < count; ++i) {
            result[i] = GenerateWord(generator, word_len);
        }
        return result;
    };
    auto strings = wordsGenerator(size, word_len);
    const auto requests = wordsGenerator(size * 2, word_len);
    std::sort(std::execution::par, strings.begin(), strings.end(), [](const auto a, const auto b) {
        return lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
    });
    vector<vector<string>::const_iterator> seq_result;
    vector<vector<string>::const_iterator> par_result;
    {
        LOG_DURATION("Последовательная версия:");
        for_each(requests.begin(), requests.end(), [&](const string_view req) {
            auto res = LowerBound(strings.begin(), strings.end(), req);
            if (res != strings.end()) {
                seq_result.push_back(res);
            }
        });
    }
    {
        LOG_DURATION("Параллельная версия:");
        for_each(requests.begin(), requests.end(), [&](const string_view req) {
            auto res = LowerBound(execution::par, strings.begin(), strings.end(), req);
            /*if (!seq_result.empty() && res != *prev(seq_result.end())) {
                auto val = *prev(seq_result.end());
                throw val;
            }*/
            if (res != strings.end()) {
                par_result.push_back(res);
            }
        });
    }

    cerr << endl;
    cerr << "Последовательная версия: " << seq_result.size() << " шт." << endl;
    cerr << "Параллельная версия: " << par_result.size() << " шт." << endl;
}

int main() {
    const vector<string> strings = {"a", "cat", "cat", "dog", "dog", "horse"};

    const vector<string> requests = {"as", "cats", "dogs", "bear", "cat", "deer", "dog", "horses"};

    // последовательные версии
    {
        for (const auto& request : requests) {
            cout << "Request [" << request << "] → position " << LowerBound(strings.begin(), strings.end(), request) - strings.begin() << endl;
        }
        cout << endl;
    }

    // параллельные
    {
        [[maybe_unused]] int i = 0;
        for (const auto& request : requests) {
            // if (i++ < 3) continue;
            cout << "Request [" << request << "] → position " << LowerBound(execution::par, strings.begin(), strings.end(), request) - strings.begin()
                 << endl;
        }
    }
    cout << endl << endl;
    Test();
    cout << endl << endl;
}