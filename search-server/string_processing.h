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

template <typename Iterator>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(Iterator begin, Iterator end) {
    std::set<std::string, std::less<>> non_empty_strings;
    std::for_each(begin, end, [&non_empty_strings](const auto str) {
        if (!str.empty()) {
            non_empty_strings.emplace(str);
        }
    });
    return non_empty_strings;
}

template <template <typename...> class Container, typename T, std::enable_if_t<std::is_convertible<T, std::string_view>::value, bool> = true>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const Container<T>& strings) {
    return MakeUniqueNonEmptyStrings(strings.begin(), strings.end());
}

/// Splits a raw text string into list of space-separated words
std::vector<std::string_view> SplitIntoWords(std::string_view text);

std::string JoinWithExclude(const std::set<std::string>& strings, const std::set<std::string>& exclude_words, const std::string& separator = ",");

template <typename T>
std::string JoinWithExclude(const std::map<std::string_view, T>& strings, const std::set<std::string>& exclude_words,
                            const std::string& separator = ",", std::shared_ptr<std::function<std::string(const std::string&)>> preprocessor = nullptr) {
    std::string result;
    std::string sep = "";
    const bool exclude_words_is_empty = exclude_words.empty();
    for (const auto& [str_view, _] : strings) {
        std::string str = static_cast<const std::string>(str_view);
        if (exclude_words_is_empty || !exclude_words.count(str)) {
            result += sep + (preprocessor != nullptr ? (*preprocessor)(str) : str);
            if (sep.empty() && !separator.empty()) {
                sep = separator;
            }
        }
    }
    return result;
}

size_t BuildHash(const std::string& str);

size_t BuildHash(const std::set<std::string>& strings, const std::set<std::string>& exclude_words, const std::string& separator = ",");

template <typename T>
size_t BuildHash(const std::map<std::string_view, T>& strings, const std::set<std::string>& exclude_words, const std::string& separator = ",",
                 std::shared_ptr<std::function<std::string(const std::string&)>> preprocessor = nullptr) {
    std::string str = JoinWithExclude(strings, exclude_words, separator, preprocessor);
    return std::hash<std::string>{}(str);
}

template <typename ExecutionPolicy>
size_t CountWords(ExecutionPolicy&& policy, std::string_view str) {
    if (str.empty()) {
        return 0;
    }
    auto ptr = std::find_if(str.begin(), str.end(), [](const char c) {
        return c != ' ';
    });
    if (ptr == str.end()) {
        return 0;
    }
    size_t size = std::transform_reduce(policy, ptr, std::prev(str.end()), std::next(ptr), 0ul, std::plus<>{},
                                        [](const char cur, const char next) {
                                            return cur != ' ' && next == ' ';
                                        }) +
                  (*std::prev(str.end()) != ' ');
    return size;
}

size_t CountWords(std::string_view str);

inline std::string ToString(const std::string_view str_view) {
    return std::string(str_view.data(), str_view.size());
}

template <template <typename...> class Container, typename T, std::enable_if_t<std::is_same<T, std::string_view>::value, bool> = true>
Container<std::string> ToString(const Container<T>& str_views) {
    std::vector<std::string> result;
    result.reserve(str_views.size());
    std::for_each(std::execution::seq, str_views.begin(), str_views.end(), [&result](const std::string_view view) {
        result.push_back(ToString(view));
    });
    return result;
}