#pragma once

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace std;

template <typename StringContainer>
extern set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

/// Splits a raw text string into list of space-separated words
vector<string> SplitIntoWords(const string& text);

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