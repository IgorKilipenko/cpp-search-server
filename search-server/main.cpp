#include <algorithm>
#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "string_processing.h"

using namespace std;

struct Stats {
    map<string, int> word_frequences;

    void operator+=(const Stats& other) {
        const auto& other_frequences = other.word_frequences;
        std::for_each(other_frequences.begin(), other_frequences.end(), [this](const auto& item) {
            word_frequences[item.first] += item.second;
        });
    }
};

using KeyWords = set<string, less<>>;

Stats ExploreKeyWords(const KeyWords& key_words, istream& input) {
    vector<string> lines;
    while (true) {
        string str;
        std::getline(input, str);
        if (str.empty()) {
            break;
        }
        lines.push_back(str);
    }

    size_t thread_count = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4ul;
    vector<std::future<Stats>> actions;
    int actions_per_thread = max(static_cast<size_t>(lines.size() / thread_count), 1ul);
    for (size_t i = 0; i < thread_count; ++i) {
        auto begin = lines.begin() + (i * actions_per_thread);
        auto end = (lines.end() - begin > actions_per_thread) ? begin + actions_per_thread : lines.end();
        actions.push_back(std::async(
            [&key_words](vector<string>::iterator begin, vector<string>::iterator end) {
                Stats stats;
                std::for_each(begin, end, [&key_words, &stats](const string_view str) {
                    const auto words = SplitIntoWords(str);
                    for (const string_view word : words) {
                        auto ptr = key_words.find(word);
                        if (ptr != key_words.end()) {
                            ++stats.word_frequences[*ptr];
                        }
                    }
                });
                return stats;
            },
            begin, end));
        if (end == lines.end()) {
            break;
        }
    }

    Stats stats;
    for (size_t i = 0; i < actions.size(); ++i) {
        stats += actions[i].get();
    }

    return stats;
}

int main() {
    const KeyWords key_words = {"yangle", "rocks", "sucks", "all"};

    stringstream ss;
    ss << "this new yangle service really rocks\n";
    ss << "It sucks when yangle isn't available\n";
    ss << "10 reasons why yangle is the best IT company\n";
    ss << "yangle rocks others suck\n";
    ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

    for (const auto& [word, frequency] : ExploreKeyWords(key_words, ss).word_frequences) {
        cout << word << " " << frequency << endl;
    }

    return 0;
}