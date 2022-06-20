#ifndef SYNONYMS_HPP
#define SYNONYMS_HPP

#include <map>
#include <set>
#include <sstream>
#include <string>

using namespace std;

class Synonyms {
   public:
    void Add(const string& first_word, const string& second_word);

    size_t GetSynonymCount(const string& word) const;

    bool AreSynonyms(const string& first_word, const string& second_word) const;

   private:
    map<string, set<string>> synonyms_;
};

#endif /* SYNONYMS_HPP */
