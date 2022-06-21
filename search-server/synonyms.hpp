#ifndef __SYNONYMS_H__
#define __SYNONYMS_H__

#include <map>
#include <set>
#include <sstream>
#include <string>

using namespace std;

class Synonyms {
   public:
    void Add(const string& first_word, const string& second_word);

    size_t GetSynonymCount(const string& word) const;

    /**
     * @brief Execute test for synonyms
     *
     * @param first_word
     * @param second_word
     * @return true if the word is synonyms
     * @return false otherwise
     */
    bool AreSynonyms(const string& first_word, const string& second_word) const;

   private:
    map<string, set<string>> synonyms_;
};

#endif  // __SYNONYMS_H__
