#include <algorithm>
#include <cstddef>
#include <execution>
#include <future>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "log_duration.h"

using namespace std;

static string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}

template <template <typename...> typename Container = list, typename T = string>
static Container<T> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<T> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    return Container(words.begin(), words.end());
}

struct Reverser {
    void operator()(string& value) const {
        reverse(value.begin(), value.end());
    }
};

template <typename Container, typename Function>
void Test(string_view mark, Container keys, Function function) {
    LOG_DURATION(mark);
    function(keys, Reverser{});
}

#define TEST(function) Test(#function, keys, function<remove_const_t<decltype(keys)>, Reverser>)

template <typename ForwardRange, typename Function>
void ForEachSync(ForwardRange& range, Function function) {
    // ускорьте эту реализацию
    for_each(execution::par, range.begin(), range.end(), function);
}

template <typename ForwardRange, typename Function>
void ForEach(ForwardRange& range, Function function) {
    using Iterator = decltype(range.begin());
    size_t size = range.size();
    size_t thread_count = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4ul;
    int items_per_thread = max(static_cast<size_t>(size * 2 / thread_count), 1ul);
    thread_count = size % items_per_thread ? size / items_per_thread + 1 : size / items_per_thread;
    vector<Iterator> args;
    args.reserve(items_per_thread);
    vector<future<void>> actions;
    actions.reserve(thread_count);
    for (auto ptr = range.begin(); ptr != range.end(); ++ptr) {
        args.push_back(ptr);
        if (args.size() % items_per_thread == 0 || ptr == prev(range.end())) {
            actions.push_back(async([args, function]() {
                for (auto ptr : args) {
                    function(*ptr);
                }
            }));
            args.clear();
        }
    }
    for (auto& action : actions) {
        action.get();
    }
}

template <typename ForwardRange, typename Function>
void ForEachYandex(ForwardRange& range, Function function) {
    static constexpr int PART_COUNT = 4;
    const auto part_length = range.size() / PART_COUNT;
    auto part_begin = range.begin();
    auto part_end = next(part_begin, part_length);

    vector<future<void>> futures;
    for (int i = 0; i < PART_COUNT; ++i, part_begin = part_end, part_end = (i == PART_COUNT - 1 ? range.end() : next(part_begin, part_length))) {
        futures.push_back(async([function, part_begin, part_end] {
            for_each(part_begin, part_end, function);
        }));
    }
}

int main() {
    // для итераторов с произвольным доступом тоже должно работать
    vector<string> strings = {"cat", "dog", "code"};

    ForEachSync(strings, [](string& s) {
        reverse(s.begin(), s.end());
    });
    ForEach(strings, [](string& s) {
        reverse(s.begin(), s.end());
    });
    ForEachYandex(strings, [](string& s) {
        reverse(s.begin(), s.end());
    });

    for (string_view s : strings) {
        cout << s << " ";
    }
    cout << endl;
    // вывод: tac god edoc

    mt19937 generator;
    const auto keys = GenerateDictionary<list>(generator, 50'000, 5'000);

    TEST(ForEachSync);
    TEST(ForEach);
    TEST(ForEachYandex);

    return 0;
}