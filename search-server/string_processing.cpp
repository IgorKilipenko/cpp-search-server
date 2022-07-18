#include "string_processing.h"

#include <pstl/glue_execution_defs.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <execution>
#include <iterator>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

vector<string_view> SplitIntoWords(string_view str) {
    if (str.empty()) {
        return {};
    }
    vector<string_view> result;
    str.remove_prefix(min(str.find_first_not_of(' '), str.size()));

    do {
        int64_t space = str.find(' ', 0);
        result.push_back(space == str.npos ? str.substr(0) : str.substr(0, space));
        str.remove_prefix(min(str.find_first_not_of(' ', space), str.size()));
    } while (!str.empty());

    /*std::mutex mutex;
    vector<int> actions(str.size());
    std::any_of(std::execution::par, std::make_move_iterator(actions.begin()), std::make_move_iterator(actions.end()), [&mutex, &str, &result](int _)
    { int64_t space = str.find(' ', 0);
        {
            std::lock_guard<std::mutex> guard(mutex);
            result.push_back(space == pos_end ? str.substr(0) : str.substr(0, space));
        }
        str.remove_prefix(min(str.find_first_not_of(' ', space), str.size()));
        return str.empty();
    });*/
    return result;
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