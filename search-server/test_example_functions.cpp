#include "test_example_functions.h"

#include <cassert>
#include <execution>
#include <iostream>
#include <random>
#include <string>

#include "document.h"
#include "log_duration.h"
#include "search_server.h"
#include "test_framework.h"

using namespace std;

string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob) {
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count, double minus_prob) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count, minus_prob));
    }
    return queries;
}

void TestParRemoveDocument() {
    SearchServer search_server("and with"s);

    int id = 0;
    for (const string& text : {
             "funny pet and nasty rat"s,
             "funny pet with curly hair"s,
             "funny pet and not very nasty rat"s,
             "pet with rat and rat and rat"s,
             "nasty rat with curly hair"s,
         }) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const string query = "curly and funny"s;

    auto report = [&search_server, &query] {
        cout << search_server.GetDocumentCount() << " documents total, "s << search_server.FindTopDocuments(query).size() << " documents for query ["s
             << query << "]"s << endl;
    };

    report();
    // однопоточная версия
    search_server.RemoveDocument(5);
    report();
    // однопоточная версия
    search_server.RemoveDocument(execution::seq, 1);
    report();
    // многопоточная версия
    search_server.RemoveDocument(execution::par, 2);
    report();
}

void TestParMatchDocument() {
    SearchServer search_server("and with"s);

    int id = 0;
    for (const string& text : {
             "funny pet and nasty rat"s,
             "funny pet with curly hair"s,
             "funny pet and not very nasty rat"s,
             "pet with rat and rat and rat"s,
             "nasty rat with curly hair"s,
         }) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const string query = "curly and funny -not"s;

    {
        const auto [words, status] = search_server.MatchDocument(query, 1);
        cout << words.size() << " words for document 1"s << endl;
        ASSERT(words.size() == 1);
        // 1 words for document 1
    }

    {
        const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
        cout << words.size() << " words for document 2"s << endl;
        ASSERT(words.size() == 2);
        // 2 words for document 2
    }

    {
        const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
        cout << words.size() << " words for document 3"s << endl;
        ASSERT(words.size() == 0);
        // 0 words for document 3
    }
}

void TestParFindTopDocuments() {
    SearchServer search_server("and with"s);

    int id = 0;
    for (const string& text : {
             "white cat and yellow hat"s,
             "curly cat curly tail"s,
             "nasty dog with big eyes"s,
             "nasty pigeon john"s,
         }) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    cout << "ACTUAL by default:"s << endl;
    // последовательная версия
    {
        auto docs = search_server.FindTopDocuments("curly nasty cat"s);
        vector<int> expected_ids{2, 4, 1, 3};
        ASSERT_EQUAL(expected_ids.size(), docs.size());
        auto expected_ptr = expected_ids.begin();
        for (auto ptr = docs.begin(); ptr != docs.end(); ++ptr, ++expected_ptr) {
            PrintDocument(*ptr);
            ASSERT_EQUAL(ptr->id, *expected_ptr);
        }
    }
    cout << "BANNED:"s << endl;
    // последовательная версия
    {
        auto docs = search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED);
        for (const Document& document : docs) {
            PrintDocument(document);
            ASSERT(docs.empty());
        }
    }

    cout << "Even ids:"s << endl;
    // параллельная версия
    {
        auto docs = search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) {
            return document_id % 2 == 0;
        });
        vector<int> expected_ids{2, 4};
        ASSERT_EQUAL(expected_ids.size(), docs.size());
        auto expected_ptr = expected_ids.begin();
        for (auto ptr = docs.begin(); ptr != docs.end(); ++ptr, ++expected_ptr) {
            PrintDocument(*ptr);
            ASSERT_EQUAL(ptr->id, *expected_ptr);
        }
    }
}