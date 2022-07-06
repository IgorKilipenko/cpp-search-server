#include "string_processing.h"

#include <string>
#include <vector>

using namespace std;

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
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