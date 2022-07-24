#include <pstl/glue_execution_defs.h>

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
/*
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
*/

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
/*
template <typename ExecutionPolicy, typename ForwardRange, typename Function,
          std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                               std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                           bool> = true>
void ForEachStandart(ExecutionPolicy&& policy, ForwardRange& range, Function function) {
    for_each(policy, range.begin(), range.end(), function);
}*/

template <typename ExecutionPolicy, typename ForwardRange, typename Function,
          std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                               std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                           bool> = true>
void ForEach([[maybe_unused]]ExecutionPolicy&& policy, ForwardRange& range, Function function) {
    using Iterator = decltype(range.begin());
    if constexpr (is_same_v<typename iterator_traits<Iterator>::iterator_category, random_access_iterator_tag> ||
                  is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value) {
        for_each(policy, range.begin(), range.end(), function);
        return;
    }
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
void ForEach(ForwardRange& range, Function function) {
    ForEach(std::execution::seq, range, function);
}

template <typename Strings>
void PrintStrings(const Strings& strings) {
    for (string_view s : strings) {
        cout << s << " ";
    }
    cout << endl;
}

int main() {
    auto reverser = [](string& s) {
        reverse(s.begin(), s.end());
    };

    list<string> strings_list = {"cat", "dog", "code"};

    ForEach(strings_list, reverser);
    PrintStrings(strings_list);
    // tac god edoc

    ForEach(execution::seq, strings_list, reverser);
    PrintStrings(strings_list);
    // cat dog code

    // единственный из вызовов, где должна работать ваша версия
    // из предыдущего задания
    ForEach(execution::par, strings_list, reverser);
    PrintStrings(strings_list);
    // tac god edoc

    vector<string> strings_vector = {"cat", "dog", "code"};

    ForEach(strings_vector, reverser);
    PrintStrings(strings_vector);
    // tac god edoc

    ForEach(execution::seq, strings_vector, reverser);
    PrintStrings(strings_vector);
    // cat dog code

    ForEach(execution::par, strings_vector, reverser);
    PrintStrings(strings_vector);
    // tac god edoc

    return 0;
}