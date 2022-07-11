#include <cassert>
#include <cstddef>
#include <iostream>
#include <string_view>
#include <vector>

using namespace std;

vector<string_view> SplitIntoWordsView(string_view str) {
    if (str.empty()) {
        return {};
    }
    vector<string_view> result;
    const int64_t pos_end = str.npos;
    str.remove_prefix(min(str.find_first_not_of(" "), str.size()));

    while (!str.empty()) {
        int64_t space = str.find(' ', 0);
        result.push_back(space == pos_end ? str.substr(0) : str.substr(0, space));
        str.remove_prefix(min(str.find_first_not_of(" ", space), str.size()));
    }

    return result;
}

int main() {
    assert((SplitIntoWordsView("") == vector<string_view>{}));
    assert((SplitIntoWordsView("     ") == vector<string_view>{}));
    assert((SplitIntoWordsView("aaaaaaa") == vector{"aaaaaaa"sv}));
    assert((SplitIntoWordsView("a") == vector{"a"sv}));
    assert((SplitIntoWordsView("a b c") == vector{"a"sv, "b"sv, "c"sv}));
    assert((SplitIntoWordsView("a    bbb   cc") == vector{"a"sv, "bbb"sv, "cc"sv}));
    assert((SplitIntoWordsView("  a    bbb   cc") == vector{"a"sv, "bbb"sv, "cc"sv}));
    assert((SplitIntoWordsView("a    bbb   cc   ") == vector{"a"sv, "bbb"sv, "cc"sv}));
    assert((SplitIntoWordsView("  a    bbb   cc   ") == vector{"a"sv, "bbb"sv, "cc"sv}));
    cout << "All OK" << endl;
}