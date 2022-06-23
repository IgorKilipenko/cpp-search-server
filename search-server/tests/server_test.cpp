#include "server_test.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

static const bool TRACE_DEBUG = true;

struct RawDocument {
    int id;
    string content;
    DocumentStatus status;
    vector<int> ratings;
};

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        assert(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        assert(doc0.id == doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        assert(server.FindTopDocuments("in"s).empty());
    }

    TRACE_DEBUG&& cout << "test: [TestExcludeStopWordsFromAddedDocumentContent] completed successfully" << endl;
}

void TestSetStopWords() {
    {
        const string stop_words = "in the stops"s;
        SearchServer server;
        server.SetStopWords(stop_words);

        const auto stop_words_map = server.GetStopWords();

        assert(!stop_words_map.empty());
        assert(stop_words_map.size() == 3);
        assert(stop_words_map.count("in"s));
        assert(stop_words_map.count("the"s));
        assert(stop_words_map.count("stops"s));
    }

    TRACE_DEBUG&& cout << "test: [TestSetStopWords] completed successfully" << endl;
}

void TestAddDocument() {
    vector<RawDocument> documents{{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                  {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                  {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                  {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}}};
    {
        SearchServer server;
        int expected_count = 0;
        assert(server.GetDocumentCount() == expected_count);

        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
            assert(server.GetDocumentCount() == ++expected_count);
        }
    }

    TRACE_DEBUG&& cout << "test: [TestAddDocument] completed successfully" << endl;
}

void TestMatchDocuments() {
    vector<RawDocument> documents{{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                  {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                  {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                  {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}}};
    SearchServer server;
    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
    }

    // Without minus_words test
    {
        string raw_query = "пушистый ухоженный кот"s;
        for (const auto& doc : documents) {
            const auto& [matched_words, status] = server.MatchDocument(raw_query, doc.id);

            assert(status == doc.status);  // Check status

            auto query_words = SplitIntoWords(raw_query);
            const int mathed_words_count = count_if(matched_words.begin(), matched_words.end(), [&query_words](const auto& mw) {
                int _count = count(query_words.begin(), query_words.end(), mw);
                assert(_count > 0);  // Mathed word must contained in query
                return _count ? 1 : 0;
            });

            auto doc_words = SplitIntoWords(doc.content);
            const int expected_mathed_words_count = count_if(query_words.begin(), query_words.end(), [&doc_words](const auto& qw) {
                int _count = count(doc_words.begin(), doc_words.end(), qw);
                return _count ? 1 : 0;
            });

            assert(expected_mathed_words_count == mathed_words_count);
        }
    }

    // With minus_words test
    {
        string expected_minus_word = "кот"s;
        string raw_query = "пушистый ухоженный кот"s + " -" + expected_minus_word;

        for (const auto& doc : documents) {
            const auto& [matched_words, status] = server.MatchDocument(raw_query, doc.id);

            assert(status == doc.status);                                                     // Check status
            assert(!count(matched_words.begin(), matched_words.end(), expected_minus_word));  // Check for matched_words not contained minus_words

            auto query_words = SplitIntoWords(raw_query);
            query_words.erase(query_words.end() - 1);  // Remove minus_word

            const int mathed_words_count = count_if(matched_words.begin(), matched_words.end(), [&query_words](const auto& mw) {
                int _count = count(query_words.begin(), query_words.end(), mw);
                assert(_count > 0);  // Mathed word must contained in query
                return _count ? 1 : 0;
            });

            auto doc_words = SplitIntoWords(doc.content);
            bool is_contained_minus_words = count(doc_words.begin(), doc_words.end(), expected_minus_word);
            const int expected_mathed_words_count =
                is_contained_minus_words ? 0 : count_if(query_words.begin(), query_words.end(), [&doc_words](const auto& qw) {
                    int _count = count(doc_words.begin(), doc_words.end(), qw);
                    return _count ? 1 : 0;
                });

            assert(expected_mathed_words_count == mathed_words_count);
        }
    }

    TRACE_DEBUG&& cout << "test: [TestMatchDocuments] completed successfully" << endl;
}

void TestSearchServer() {
    TRACE_DEBUG&& cout << "*** Start SearchServer testings..." << endl << endl;

    TestExcludeStopWordsFromAddedDocumentContent();

    TestSetStopWords();

    TestAddDocument();

    TestMatchDocuments();

    TRACE_DEBUG&& cout << endl << "*** SearchServer testings completed." << endl;
}
