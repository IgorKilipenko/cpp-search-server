#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

template <template <typename...> class Container, typename T, std::enable_if_t<std::is_same<T, string>::value, bool> = true> 
set<string> MakeUniqueNonEmptyStrings(const Container<T>& strings) {
    set<string> non_empty_strings;
    for (const T& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

template <template <typename...> class Container, typename T, std::enable_if_t<std::is_same<T, string_view>::value, bool> = true>
//requires std::convertible_to<T, string_view>
set<string> MakeUniqueNonEmptyStrings(const Container<T>& strings) {
    set<string> non_empty_strings;
    for (const T str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(string(str));
        }
    }
    return non_empty_strings;
}

/// Splits a raw text string into list of space-separated words
vector<string_view> SplitIntoWords(string_view text);

string JoinWithExclude(const set<string>& strings, const set<string>& exclude_words, string separator = ","s);

template <typename T>
string JoinWithExclude(const map<string, T>& strings, const set<string>& exclude_words, string separator = ","s,
                       shared_ptr<function<string(const string&)>> preprocessor = nullptr) {
    string result;
    string sep = "";
    const bool exclude_words_is_empty = exclude_words.empty();
    for (const auto& [str, _] : strings) {
        if (exclude_words_is_empty || !exclude_words.count(str)) {
            result += sep + (preprocessor != nullptr ? (*preprocessor)(str) : str);
            if (sep.empty() && !separator.empty()) {
                sep = separator;
            }
        }
    }
    return result;
}

size_t BuildHash(const string& str);

size_t BuildHash(const set<string>& strings, const set<string>& exclude_words, string separator = ","s);

template <typename T>
size_t BuildHash(const map<string, T>& strings, const set<string>& exclude_words, string separator = ","s,
                 shared_ptr<function<string(const string&)>> preprocessor = nullptr) {
    string str = JoinWithExclude(strings, exclude_words, separator, preprocessor);
    return hash<string>{}(str);
}

template <typename ExecutionPolicy>
size_t CountWords(ExecutionPolicy&& policy, string_view str) {
    if (str.empty()) {
        return 0;
    }
    auto ptr = std::find_if(str.begin(), str.end(), [](const char c) {
        return c != ' ';
    });
    if (ptr == str.end()) {
        return 0;
    }
    size_t size = std::transform_reduce(policy, ptr, prev(str.end()), next(ptr), 0ul, std::plus<>{},
                                        [](const char cur, const char next) {
                                            return cur != ' ' && next == ' ';
                                        }) +
                  (*prev(str.end()) != ' ');
    return size;
}

size_t CountWords(string_view str);