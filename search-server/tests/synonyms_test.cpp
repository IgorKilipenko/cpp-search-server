#include <cassert>

#include "tests.hpp"

extern void TestAddingSynonymsIncreasesTheirCount() {
    Synonyms synonyms;
    assert(synonyms.GetSynonymCount("music"s) == 0);
    assert(synonyms.GetSynonymCount("melody"s) == 0);

    synonyms.Add("music"s, "melody"s);
    assert(synonyms.GetSynonymCount("music"s) == 1);
    assert(synonyms.GetSynonymCount("melody"s) == 1);

    synonyms.Add("music"s, "tune"s);
    assert(synonyms.GetSynonymCount("music"s) == 2);
    assert(synonyms.GetSynonymCount("tune"s) == 1);
    assert(synonyms.GetSynonymCount("melody"s) == 1);
}

extern void TestAreSynonyms() {
    // Напишите недостающий код
    Synonyms synonyms;
    
    string first_word = "music";
    string second_word = "music";
    synonyms.Add("music"s, "melody"s);
    assert(synonyms.AreSynonyms(first_word, second_word));
}