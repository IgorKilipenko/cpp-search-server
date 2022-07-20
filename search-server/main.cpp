#include <algorithm>
#include <cstddef>
#include <execution>
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

void Test() {
    size_t size = 1'000'000;
    size_t word_len = 1000;
    const auto wordsGenerator = [](size_t count, size_t word_len) {
        mt19937 generator;
        vector<string> result{count};
        for (size_t i = 0; i < count; ++i) {
            result[i] = GenerateWord(generator, word_len);
        }
        return result;
    };
    const auto strings = wordsGenerator(size, word_len);
    const auto requests = wordsGenerator(size * 2, word_len);
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
            auto res = LowerBound(strings.begin(), strings.end(), req);
            if (res != strings.end()) {
                par_result.push_back(res);
            }
        });
    }

    cerr << endl;
    cerr << "Последовательная версия: " << seq_result.size() << " шт." << endl;
    cerr << "Последовательная версия: " << par_result.size() << " шт." << endl;
}

int main() {
    const vector<string> strings = {"cat", "dog", "dog", "horse"};

    const vector<string> requests = {"bear", "cat", "deer", "dog", "dogs", "horses"};

    // последовательные версии
    {
        cout << "Request [" << requests[0] << "] → position " << LowerBound(strings.begin(), strings.end(), requests[0]) - strings.begin() << endl;
        cout << "Request [" << requests[1] << "] → position "
             << LowerBound(execution::seq, strings.begin(), strings.end(), requests[1]) - strings.begin() << endl;
        cout << "Request [" << requests[2] << "] → position "
             << LowerBound(execution::seq, strings.begin(), strings.end(), requests[2]) - strings.begin() << endl;
    }

    // параллельные
    {
        cout << "Request [" << requests[3] << "] → position "
             << LowerBound(execution::par, strings.begin(), strings.end(), requests[3]) - strings.begin() << endl;
        cout << "Request [" << requests[4] << "] → position "
             << LowerBound(execution::par, strings.begin(), strings.end(), requests[4]) - strings.begin() << endl;
        cout << "Request [" << requests[5] << "] → position "
             << LowerBound(execution::par, strings.begin(), strings.end(), requests[5]) - strings.begin() << endl;
    }
    cout << endl << endl;
    Test();
    cout << endl << endl;
}