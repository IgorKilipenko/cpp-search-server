#include "synonyms.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

using namespace std;

void Synonyms::Add(const string& first_word, const string& second_word) {
    synonyms_[first_word].insert(second_word);
    synonyms_[second_word].insert(first_word);
}

size_t Synonyms::GetSynonymCount(const string& word) const {
    if (synonyms_.count(word) != 0) {
        return synonyms_.at(word).size();
    }
    return 0;
}

bool Synonyms::AreSynonyms(const string& first_word, const string& second_word) const {
    // Напишите недостающий код
    return synonyms_.count(first_word) && synonyms_.at(first_word).count(second_word);
}

map<string, set<string>> synonyms_;
