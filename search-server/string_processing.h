#pragma once

#include <algorithm>
#include <cstddef>
#include <execution>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

inline string ToString(const string_view str_view) {
    return string(str_view.data(), str_view.size());
}

template <template <typename...> class Container, typename T, std::enable_if_t<std::is_same<T, string_view>::value, bool> = true>
Container<string> ToString(const Container<T>& str_views) {
    vector<string> result;
    result.reserve(str_views.size());
    std::for_each(std::execution::seq, str_views.begin(), str_views.end(), [&result](const string_view view) {
        result.push_back(ToString(view));
    });
    return result;
}

template <typename Iterator>
set<string, std::less<>> MakeUniqueNonEmptyStrings(Iterator begin, Iterator end) {
    set<string, std::less<>> non_empty_strings;
    std::for_each(begin, end, [&non_empty_strings](const auto str) {
        if (!str.empty()) {
            non_empty_strings.emplace(str);
        }
    });
    return non_empty_strings;
}

template <template <typename...> class Container, typename T, std::enable_if_t<std::is_convertible<T, std::string_view>::value, bool> = true>
set<string, std::less<>> MakeUniqueNonEmptyStrings(const Container<T>& strings) {
    return MakeUniqueNonEmptyStrings(strings.begin(), strings.end());
}

/*
template <template <typename...> class Container, typename T, std::enable_if_t<std::is_same<T, string_view>::value, bool> = true>
set<string> MakeUniqueNonEmptyStrings(const Container<T>& strings) {
    return MakeUniqueNonEmptyStrings(ToString(strings));
}*/

/// Splits a raw text string into list of space-separated words
vector<string_view> SplitIntoWords(string_view text);

string JoinWithExclude(const set<string>& strings, const set<string>& exclude_words, string separator = ","s);

template <typename T>
string JoinWithExclude(const map<string_view, T>& strings, const set<string>& exclude_words, string separator = ","s,
                       shared_ptr<function<string(const string&)>> preprocessor = nullptr) {
    string result;
    string sep = "";
    const bool exclude_words_is_empty = exclude_words.empty();
    for (const auto& [str_view, _] : strings) {
        string str = static_cast<const string>(str_view);
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
size_t BuildHash(const map<string_view, T>& strings, const set<string>& exclude_words, string separator = ","s,
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
