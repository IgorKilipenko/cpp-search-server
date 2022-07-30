#include <algorithm>
#include <atomic>
#include <cstddef>
#include <execution>
#include <iostream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

using namespace std;

template <typename Hash>
int FindCollisions(const Hash& hasher, istream& text) {
    // место для вашей реализации
    std::unordered_map<size_t, unordered_set<std::string>> words;
    size_t collisions = 0;
    string word;
    vector<std::string> input;
    while (text >> word) {
        input.push_back(std::move(word));
    }

    std::mutex mutex;
    std::for_each(std::execution::par, std::make_move_iterator(input.begin()), std::make_move_iterator(input.end()), [&](const std::string& word) {
        const auto hash = hasher(word);
        std::lock_guard lock(mutex);
        if (words.count(hash) && !words[hash].count(word)) {
            ++collisions;
        }
        words[hash].insert(std::move(word));
    });

    return collisions;
}

// Это плохой хешер. Его можно использовать для тестирования.
// Подумайте, в чём его недостаток
struct HasherDummyOld {
    size_t operator()(const string& str) const {
        size_t res = 0;
        for (char c : str) {
            res += static_cast<size_t>(c);
        }
        return res;
    }
};

struct DummyHash {
    size_t operator()(const string&) const {
        return 42;
    }
};

int main() {
    DummyHash dummy_hash;
    hash<string> good_hash;

    {
        istringstream stream("I love C++"s);
        cout << FindCollisions(dummy_hash, stream) << endl;
    }
    {
        istringstream stream("I love C++"s);
        cout << FindCollisions(good_hash, stream) << endl;
    }
}