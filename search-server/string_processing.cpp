#include "string_processing.h"

#include <pstl/glue_execution_defs.h>

#include <cassert>
#include <cstddef>
#include <execution>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

vector<string> SplitIntoWords(const string& text) {
    /*if (text.empty()) {
        return {};
    }
    size_t size = CountWords(text);
    vector<string> words;
    words.reserve(size);
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;*/

    /*
    if (text.empty()) {
        return {};
    }
    auto ptr = std::find_if(text.begin(), text.end(), [](const char c) {
        return c != ' ';
    });
    if (ptr == text.end()) {
        return {};
    }
    vector<string> words;
    auto ptr_prev = text.begin();
    for (auto ptr = text.begin(), ptr_next = next(text.begin()); ptr_next != text.end();) {
        if (*ptr != ' ' && *ptr_next == ' ') {
            words.push_back({ptr_prev, ptr_next});
            ptr_prev = std::find_if(next(ptr_next), text.end(), [](const char c) {
                return c != ' ';
            });
            ptr = prev(ptr_prev);
            ptr_next = ptr_prev;
        }
        ++ptr;
        ++ptr_next;
    }
    if (*prev(text.end()) != ' ') {
        words.push_back(string(ptr_prev, text.end()));
    }
    return words;
    */

    if (text.empty()) {
        return {};
    }
    /*auto ptr = std::find_if(text.begin(), text.end(), [](const char c) {
        return c != ' ';
    });
    if (ptr == text.end()) {
        return {};
    }*/
    string_view str{text};
    vector<string> words;

    for (int idx = 0; /*text[idx] != string::npos*/;) {
        if (str.empty()) {
            break;
        }
        idx = str.find_first_not_of(' ');
        if (idx < 0) {
            break;
        }
        str = str.substr(idx);
        int next_idx = str.find(' ');
        next_idx = next_idx < 0 ? str.length() : next_idx;
        words.push_back(string(str.substr(0, next_idx)));
        str = str.substr(next_idx);
    }
    return words;
}

size_t BuildHash(const string& str) {
    return hash<string>{}(str);
}

size_t BuildHash(const set<string>& strings, const set<string>& exclude_words, string separator) {
    string str = JoinWithExclude(strings, exclude_words, separator);
    return hash<string>{}(str);
}

string JoinWithExclude(const set<string>& strings, const set<string>& exclude_words, string separator) {
    string result;
    string sep = "";
    const bool exclude_words_is_empty = exclude_words.empty();
    for (const string& str : strings) {
        if (exclude_words_is_empty || !exclude_words.count(str)) {
            result += sep + str;
            if (sep.empty() && !separator.empty()) {
                sep = separator;
            }
        }
    }
    return result;
}

size_t CountWords(string_view str) {
    return CountWords(std::execution::seq, str);
}