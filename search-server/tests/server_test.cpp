#include "server_test.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>


/* HELPERS region ---------------------------------------------------------------- */

struct RawDocument {
    int id;
    string content;
    DocumentStatus status;
    vector<int> ratings;
};

static shared_ptr<Document> getDocumentById(int id, vector<Document> documents) {
    const shared_ptr<Document> result =
        accumulate(documents.begin(), documents.end(), shared_ptr<Document>{nullptr}, [id](shared_ptr<Document> curr, const Document& doc) {
            if (doc.id == id) {
                curr = make_shared<Document>(doc);
            }
            return curr;
        });
    return result;
}

static bool equalDocuments(const Document& d1, const Document d2) {
    const double relevance_threshold = 0.000001;
    return d1.id == d2.id && (abs(d1.relevance - d2.relevance) < relevance_threshold) && d1.rating == d2.rating;
}

template <typename T>
static set<T> toSet(const vector<T>& values) {
    set<T> result(values.begin(), values.end());
    return result;
}

template <typename T>
void ExpectTest(T (*test)(bool), int& test_number, bool TRACE_DEBUG = false) {
    TRACE_DEBUG&& cout << endl << "Starting [test #"s << ++test_number << "]..." << endl;
    test(TRACE_DEBUG);
    TRACE_DEBUG&& cout << "[Test #"s << test_number << "] completed successfully."s << endl;
}

/* ---------------------------------------------------------------- */

static const vector<RawDocument> initial_documents{{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                                   {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                                   {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                                   {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}},
                                                   {4, ""s, DocumentStatus::ACTUAL, {0}}};

void TestSplitWords(bool TRACE_DEBUG) {
    const string TAG = "TestSplitWords"s;
    const string stop_words = "   word   word1  word2    "s;
    const auto stops = SplitIntoWords(stop_words);
    assert(stops.size() == 3 && stops[0] == "word" && stops[2] == "word2");

    TRACE_DEBUG&& cout << "test: ["s << TAG << "] completed successfully" << endl;
}

/// Поддержка стоп-слов. Стоп-слова исключаются из текста документов.
void TestExcludeStopWordsFromAddedDocumentContent(bool TRACE_DEBUG) {
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

/**
 * @brief Поддержка стоп-слов. Стоп-слова исключаются из текста документов.
 * Альтернативный вариант реализации
 */
void TestStopWords(bool TRACE_DEBUG) {
    const string TAG = "TestStopWords"s;
    {
        const string stop_words = "word word1 word2"s;
        const set<string> stop_words_set = toSet(SplitIntoWords(stop_words));
        SearchServer server;
        server.SetStopWords(stop_words);
        vector<RawDocument> documents = {{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                         {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                         {2, "ухоженный пёс выразительные глаза word"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                         {3, "ухоженный скворец евгений word"s, DocumentStatus::ACTUAL, {9}},
                                         {4, "ухоженный скворец евгений word1"s, DocumentStatus::ACTUAL, {9}},
                                         {5, "ухоженный скворец евгений word2"s, DocumentStatus::ACTUAL, {9}},
                                         {6, "   ухоженный скворец евгений word word1 word2     "s, DocumentStatus::ACTUAL, {9}},
                                         {7, stop_words, DocumentStatus::ACTUAL, {0}},
                                         {8, ""s, DocumentStatus::ACTUAL, {0}}};

        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }
        {
            const auto found_docs = server.FindTopDocuments(" "s);
            assert(found_docs.size() == 0);
        }
        {
            const string word = "евгений word";
            const auto found_docs = server.FindTopDocuments(word);
            assert(found_docs.size() == 4);
            for (const auto& doc : found_docs) {
                const auto& [words, status] = server.MatchDocument(word, doc.id);
                assert(status == DocumentStatus::ACTUAL);
                for (const auto& w : words) {
                    assert(!stop_words_set.count(w));
                }
            }
        }
        {
            const string word = "глаза";
            const auto found_docs = server.FindTopDocuments(word);
            assert(found_docs.size() == 1);
            for (const auto& doc : found_docs) {
                const auto& [words, status] = server.MatchDocument(word, doc.id);
                assert(status == DocumentStatus::ACTUAL);
                for (const auto& w : words) {
                    assert(!stop_words_set.count(w));
                }
            }
        }
        {
            const string word = "ухоженный";
            const auto found_docs = server.FindTopDocuments(word);
            assert(found_docs.size() == 5);
            for (const auto& doc : found_docs) {
                const auto& [words, status] = server.MatchDocument(word, doc.id);
                assert(status == DocumentStatus::ACTUAL);
                for (const auto& w : words) {
                    assert(!stop_words_set.count(w));
                }
            }
        }

        // Loop for all stop words
        {
            for (const auto& sw : stop_words_set) {
                const auto& found_docs = server.FindTopDocuments(sw);
                for (const auto& doc : found_docs) {
                    const auto& [words, status] = server.MatchDocument(sw, doc.id);
                    assert(status == DocumentStatus::ACTUAL);
                    for (const auto& w : words) {
                        assert(!stop_words_set.count(w));
                    }
                }
            }
            TRACE_DEBUG&& cout << "test: ["s << TAG << " | Loop for all stop words] completed successfully" << endl;
        }
    }

    TRACE_DEBUG&& cout << "test: ["s << TAG << "] completed successfully" << endl;
}

/**
 * @brief Поддержка минус-слов.
 * Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
 * @param TRACE_DEBUG
 */
void TestMinusWords(bool TRACE_DEBUG) {
    const string TAG = "TestSplitWords"s;

    vector<RawDocument> documents = {{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                     {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                     {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                     {3, "ухоженный скворец евгений"s, DocumentStatus::ACTUAL, {9}},
                                     {4, "ухоженный скворец евгений"s, DocumentStatus::ACTUAL, {9}},
                                     {5, "ухоженный скворец евгений"s, DocumentStatus::ACTUAL, {9}},
                                     {6, "ухоженный скворец евгений"s, DocumentStatus::ACTUAL, {9}}};
    SearchServer server;
    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
    }
    {
        int expected_count = 1;
        int expected_id = 2;
        const string query = "ухоженный белый -ошейник -евгений"s;
        const auto& top_documents = server.FindTopDocuments(query);
        assert(top_documents.size() == expected_count && top_documents[0].id == expected_id);
        assert(!count_if(top_documents.begin(), top_documents.end(), [](const auto& doc) {
            return doc.id == 0 || doc.id > 2;
        }));
    }
    {
        const string query = "ухоженный белый кот пёс -ошейник -евгений"s;
        {
            const auto& [words, _] = server.MatchDocument(query, 0);
            assert(words.empty());
        }
        {
            const auto& [words, _] = server.MatchDocument(query, 2);
            assert(!words.empty());
        }
        for (int i = 3; i <= 6; i++) {
            const auto& [words, _] = server.MatchDocument(query, i);
            assert(words.empty());
        }
    }

    TRACE_DEBUG&& cout << "test: ["s << TAG << "] completed successfully" << endl;
}

/**
 * @brief Добавление документов.
 * Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
 */
void TestAddDocument(bool TRACE_DEBUG) {
    const string TAG = "TestAddDocument";
    vector<RawDocument> documents = initial_documents;
    SearchServer server;

    int expected_count = 0;
    assert(server.GetDocumentCount() == expected_count);

    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
        assert(server.GetDocumentCount() == ++expected_count);
    }

    // Test for document is matching
    {
        for (const auto& [id, content, status, ratings] : documents) {
            const auto& [words, mathed_status] = server.MatchDocument(content, id);
            assert(mathed_status == status);
            assert(!words.empty() || (content.empty() && words.empty()));
        }
        TRACE_DEBUG&& cout << "test: [" << TAG << " | document is matching] completed successfully" << endl;
    }

    // Test for document in tops
    {
        for (const auto& raw_doc : documents) {
            const auto& mathed_documents = server.FindTopDocuments(raw_doc.content, raw_doc.status);
            const int tops_count = count_if(mathed_documents.begin(), mathed_documents.end(), [&raw_doc](const auto& document) {
                return document.id == raw_doc.id;
            });
            assert(tops_count || (raw_doc.content.empty() && tops_count == 0));
        }
        TRACE_DEBUG&& cout << "test: [" << TAG << " | document in tops] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Матчинг документов.
 * При матчинге документа по поисковому запросу должны быть возвращены все слова из поискового запроса, присутствующие в документе.
 * Если есть соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
 * @param TRACE_DEBUG
 */
void TestMatchDocuments(bool TRACE_DEBUG) {
    const string TAG = "TestMatchDocuments";

    // Without minus_words test
    {
        vector<RawDocument> documents = initial_documents;
        SearchServer server;
        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }
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
        TRACE_DEBUG&& cout << "test: [" << TAG << " | Without minus_words] completed successfully" << endl;
    }

    // Simple with minus_words test
    {
        const string initial_content = "белый кот и модный ошейник"s;
        RawDocument raw_doc = {0, initial_content, DocumentStatus::ACTUAL, {8, -3}};
        vector<RawDocument> raw_documents = {raw_doc};
        SearchServer server;
        for (const auto& [id, content, status, ratings] : raw_documents) {
            server.AddDocument(id, content, status, ratings);
        }
        string expected_minus_word = "кот"s;
        string raw_query = initial_content + " -" + expected_minus_word;

        const auto& [matched_words, status] = server.MatchDocument(raw_query, raw_doc.id);
        assert(status == DocumentStatus::ACTUAL);
        assert(matched_words.empty());

        TRACE_DEBUG&& cout << "test: [" << TAG << " | With minus_words] completed successfully" << endl;
    }

    // With minus_words test
    {
        vector<RawDocument> documents = initial_documents;
        SearchServer server;
        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }
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
        TRACE_DEBUG&& cout << "test: [" << TAG << "| With minus_words] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Сортировка найденных документов по релевантности.
 * Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
 * @param TRACE_DEBUG
 */
void TestRelevanceSortOrder(bool TRACE_DEBUG) {
    const string TAG = "TestRelevanceSortOrder";

    const DocumentStatus status = DocumentStatus::ACTUAL;
    // Documents from example
    vector<RawDocument> documents = {{0, "белый кот и модный ошейник"s, status, {8, -3}},
                                     {1, "пушистый кот пушистый хвост"s, status, {7, 2, 7}},
                                     {2, "ухоженный пёс выразительные глаза"s, status, {5, -12, 2, 1}},
                                     {3, "ухоженный скворец евгений"s, status, {9}}};
    const string query = "пушистый ухоженный кот"s;

    // Without stop_words test
    {
        /*
         * Expected result [WITHOUT STOP WORDS]:
         * { document_id = 1, relevance = 0.866434, rating = 5 }
         * { document_id = 3, relevance = 0.231049, rating = 9 }
         * { document_id = 2, relevance = 0.173287, rating = -1 }
         * { document_id = 0, relevance = 0.138629, rating = 2 }
         */

        SearchServer server;
        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }
        const vector<Document> expected_documents{
            {1, 0.866434, 5},
            {3, 0.231049, 9},
            {2, 0.173287, -1},
            {0, 0.138629, 2},
        };

        const auto& result_documents = server.FindTopDocuments(query, status);

        assert(result_documents.size() == expected_documents.size());

        for (int i = 0; i < result_documents.size(); i++) {
            const Document& rd = result_documents[i];
            const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
            assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
            assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
        }
        TRACE_DEBUG&& cout << "test: [" << TAG << " | Without stop_words] completed successfully" << endl;
    }

    // With stop_words test
    {
        /*
         * Expected result [WITH STOP WORDS]:
         * { document_id = 1, relevance = 0.866434, rating = 5 }
         * { document_id = 3, relevance = 0.231049, rating = 9 }
         * { document_id = 0, relevance = 0.173287, rating = 2 }
         * { document_id = 2, relevance = 0.173287, rating = -1 }
         */

        SearchServer server;
        server.SetStopWords("и в на"s);
        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }

        const vector<Document> expected_documents{{1, 0.866434, 5}, {3, 0.231049, 9}, {0, 0.173287, 2}, {2, 0.173287, -1}};

        const auto& result_documents = server.FindTopDocuments(query);

        assert(result_documents.size() == expected_documents.size());

        for (int i = 0; i < result_documents.size(); i++) {
            const Document& rd = result_documents[i];
            const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
            assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
            assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | with stop_words] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Сортировка найденных документов по релевантности.
 * Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
 * @param TRACE_DEBUG
 */
void TestRatingSortOrder(bool TRACE_DEBUG) {
    const string TAG = "TestRatingSortOrder";

    /*
     * Expected result:
     * { document_id = 4, relevance = 0.0, rating = 10 }
     * { document_id = 3, relevance = 0.0, rating = 9 }
     * { document_id = 1, relevance = 0.0, rating = 5 }
     * { document_id = 0, relevance = 0.0, rating = 2 }
     * { document_id = 2, relevance = 0.0, rating = -1 }
     */

    const DocumentStatus status = DocumentStatus::ACTUAL;
    const string content = "word1 word2 word3"s;
    const string query = content;
    // Raw documents
    vector<RawDocument> documents = {{0, content, status, {8, -3}},
                                     {1, content, status, {7, 2, 7}},
                                     {2, content, status, {5, -12, 2, 1}},
                                     {3, content, status, {9}},
                                     {4, content, status, {19, 2}}};
    vector<Document> expected_top_documents{
        {4, 0.0, 10}, {3, 0.0, 9}, {1, 0.0, 5}, {0, 0.0, 2}, {2, 0.0, -1},
    };

    SearchServer server;
    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
    }

    const auto& matching_documents = server.FindTopDocuments(query);
    assert(matching_documents.size() >= expected_top_documents.size());

    for (int i = 0; i < expected_top_documents.size(); i++) {
        assert(matching_documents[i].id == expected_top_documents[i].id && matching_documents[i].rating == expected_top_documents[i].rating);
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Фильтрация результатов поиска.
 * Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
 * @param TRACE_DEBUG
 */
void TestFilteringWihtPredicate(bool TRACE_DEBUG) {
    const string TAG = "TestFilteringWihtPredicate";
    const string shared_word = "word"s;
    // Documents from example
    vector<RawDocument> documents = {{0, shared_word + " белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                     {1, shared_word + " пушистый кот пушистый хвост"s, DocumentStatus::BANNED, {7, 2, 7}},
                                     {2, shared_word + " ухоженный пёс выразительные глаза"s, DocumentStatus::IRRELEVANT, {5, -12, 2, 1}},
                                     {3, shared_word + " ухоженный скворец евгений"s, DocumentStatus::REMOVED, {9}}};
    SearchServer server;
    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
    }

    const string query = shared_word;
    // Strict requests
    auto strictRequests = [&]() {
        // Test ACTUAL
        {
            const int request_id = 0;
            const auto request_status = DocumentStatus::ACTUAL;
            const int request_rating = 2;

            const auto& matched_documents =
                server.FindTopDocuments(query, [request_id, request_status, request_rating](int id, DocumentStatus status, int rating) -> bool {
                    return id == request_id && status == request_status && rating == request_rating;
                });
            assert(matched_documents.size() == 1);

            const auto& matched_doc = matched_documents.front();
            assert(matched_doc.id == request_id);
            assert(matched_doc.rating == request_rating);
        }

        // Test REMOVED
        {
            const int request_id = 3;
            const auto request_status = DocumentStatus::REMOVED;
            const int request_rating = 9;

            const auto& matched_documents =
                server.FindTopDocuments(query, [request_id, request_status, request_rating](int id, DocumentStatus status, int rating) -> bool {
                    return id == request_id && status == request_status && rating == request_rating;
                });
            assert(matched_documents.size() == 1);

            const auto& matched_doc = matched_documents.front();
            assert(matched_doc.id == request_id);
            assert(matched_doc.rating == request_rating);
        }
        TRACE_DEBUG&& cout << "test: [" << TAG << " | Strict requests] completed successfully" << endl;
    };
    strictRequests();

    // Not Strict requests
    auto notStrictRequests = [&]() {
        // Test ACTUAL
        {
            // Expect success
            {
                const auto request_status = DocumentStatus::ACTUAL;
                const int request_min_rating = 1;
                const int expected_id = 0;
                const int expected_rating = 2;

                const auto& matched_documents = server.FindTopDocuments(
                    query, [request_status, request_min_rating]([[maybe_unused]] int id, DocumentStatus status, int rating) -> bool {
                        return status == request_status && rating > request_min_rating;
                    });
                assert(matched_documents.size() == 1);

                const auto& matched_doc = matched_documents.front();
                assert(matched_doc.id == expected_id);
                assert(matched_doc.rating == expected_rating);
            }

            // Expect failure
            {
                {
                    const auto request_status = DocumentStatus::ACTUAL;
                    const int request_min_rating = 3;

                    const auto& matched_documents = server.FindTopDocuments(
                        query, [request_status, request_min_rating]([[maybe_unused]] int id, DocumentStatus status, int rating) -> bool {
                            return status == request_status && rating > request_min_rating;
                        });
                    assert(matched_documents.size() == 0);
                }
                {
                    const auto request_status = DocumentStatus::REMOVED;
                    const int request_id = 0;

                    const auto& matched_documents = server.FindTopDocuments(
                        query, [request_status, request_id](int id, DocumentStatus status, [[maybe_unused]] int rating) -> bool {
                            return status == request_status && id == request_id;
                        });
                    assert(matched_documents.size() == 0);
                }
            }
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | Not Strict requests] completed successfully" << endl;
    };
    notStrictRequests();

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Поиск документов, имеющих заданный статус.
 *
 * @param TRACE_DEBUG
 */
void TestFindDocumentsBySpecifiedStatus(bool TRACE_DEBUG) {
    const string TAG = "TestFindDocumentsBySpecifiedStatus"s;

    const string shared_word = "word"s;
    const string query = shared_word;
    vector<RawDocument> documents = {
        {static_cast<int>(DocumentStatus::ACTUAL), shared_word + " белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
        {static_cast<int>(DocumentStatus::BANNED), shared_word + " пушистый кот пушистый хвост"s, DocumentStatus::BANNED, {7, 2, 7}},
        {static_cast<int>(DocumentStatus::IRRELEVANT),
         shared_word + " ухоженный пёс выразительные глаза"s,
         DocumentStatus::IRRELEVANT,
         {5, -12, 2, 1}},
        {static_cast<int>(DocumentStatus::REMOVED), shared_word + " ухоженный скворец евгений"s, DocumentStatus::REMOVED, {9}}};

    SearchServer server;
    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
    }

    for (const auto& raw_doc : documents) {
        const auto& matched_documents = server.FindTopDocuments(query, static_cast<DocumentStatus>(raw_doc.id));
        assert(matched_documents.size() == 1);
        assert(matched_documents.front().id == raw_doc.id);
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

void TestFindTopDocuments(bool TRACE_DEBUG) {
    const string TAG = "TestFindTopDocuments"s;

    // Max top count (sort by rating) documents test
    {
        const double threshold = 1e-6;
        const int documents_size = 10;
        const int expected_top_documents_count = MAX_RESULT_DOCUMENT_COUNT;

        SearchServer server;

        const string word = "word";
        vector<RawDocument> raw_documents;
        raw_documents.reserve(documents_size);

        for (int i = 0; i < documents_size; i++) {
            string content = word;
            for (int j = 0; j < min(i, 10); j++) {
                content += " " + word;
            }
            const RawDocument doc = {i, content, DocumentStatus::ACTUAL, {i, i - 1, i + 1}};
            raw_documents.push_back(doc);
            server.AddDocument(doc.id, doc.content, doc.status, doc.ratings);
        }

        const auto& matched_documents = server.FindTopDocuments(word);
        assert(matched_documents.size() == expected_top_documents_count);

        vector<RawDocument> expected_documents(raw_documents.end() - expected_top_documents_count, raw_documents.end());
        reverse(expected_documents.begin(), expected_documents.end());

        for (int i = 0; i < expected_top_documents_count; i++) {
            assert(expected_documents[i].id == matched_documents[i].id);
        }
        for (int i = 0; i < expected_top_documents_count - 1; i++) {
            const double relevance_order = matched_documents[i].relevance - matched_documents[i + 1].relevance;
            assert(relevance_order > 0 || (abs(relevance_order) < threshold && matched_documents[i].rating >= matched_documents[i + 1].rating));
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | Max top count (sort by rating)] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Корректное вычисление релевантности найденных документов.
 * 
 * @param TRACE_DEBUG 
 */
void TestCommon(bool TRACE_DEBUG) {
    const string TAG = "TestCommon"s;

    // WITHOUT STOP WORDS:
    {
        /*
         * Expected results [WITHOUT STOP WORDS]:
         * ACTUAL by default:
         * { document_id = 1, relevance = 0.866434, rating = 5 }
         * { document_id = 2, relevance = 0.173287, rating = -1 }
         * { document_id = 0, relevance = 0.138629, rating = 2 }
         * BANNED:
         * { document_id = 3, relevance = 0.231049, rating = 9 }
         * Even ids:
         * { document_id = 2, relevance = 0.173287, rating = -1 }
         * { document_id = 0, relevance = 0.138629, rating = 2 }
         */

        // Documents from example
        vector<RawDocument> documents = {{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                         {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                         {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                         {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}}};

        SearchServer server;
        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }

        // ACTUAL by default;
        {
            const vector<Document> expected_documents{
                {1, 0.866434, 5},
                {2, 0.173287, -1},
                {0, 0.138629, 2},
            };

            const auto& result_documents = server.FindTopDocuments("пушистый ухоженный кот"s);

            assert(result_documents.size() == expected_documents.size());

            for (int i = 0; i < result_documents.size(); i++) {
                const Document& rd = result_documents[i];
                const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
                assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
                assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
            }
            TRACE_DEBUG&& cout << "test: [" << TAG << " | ACTUAL by default without stop_words] completed successfully" << endl;
        }

        // BANNED
        {
            const vector<Document> expected_documents{{3, 0.231049, 9}};
            const auto& result_documents = server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);

            assert(result_documents.size() == expected_documents.size());

            for (int i = 0; i < result_documents.size(); i++) {
                const Document& rd = result_documents[i];
                const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
                assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
                assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
            }
            TRACE_DEBUG&& cout << "test: [" << TAG << " | BANNED without stop_words] completed successfully" << endl;
        }

        // Even ids
        {
            const vector<Document> expected_documents{{2, 0.173287, -1}, {0, 0.138629, 2}};
            const auto& result_documents = server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) {
                return document_id % 2 == 0;
            });

            assert(result_documents.size() == expected_documents.size());

            for (int i = 0; i < result_documents.size(); i++) {
                const Document& rd = result_documents[i];
                const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
                assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
                assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
            }
            TRACE_DEBUG&& cout << "test: [" << TAG << " | Even ids without stop_words] completed successfully" << endl;
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | without stop_words] completed successfully" << endl;
    }

    // WITH STOP WORDS:
    {
        /*
         * Expected results [WITH STOP WORDS]:
         * Expected results:
         * ACTUAL by default:
         * { document_id = 1, relevance = 0.866434, rating = 5 }
         * { document_id = 0, relevance = 0.173287, rating = 2 }
         * { document_id = 2, relevance = 0.173287, rating = -1 }
         * BANNED:
         * { document_id = 3, relevance = 0.231049, rating = 9 }
         * Even ids:
         * { document_id = 0, relevance = 0.173287, rating = 2 }
         * { document_id = 2, relevance = 0.173287, rating = -1 }
         */

        // Documents from example
        vector<RawDocument> documents = {{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                         {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                         {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                         {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}}};

        SearchServer server;
        server.SetStopWords("и в на"s);
        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }

        // ACTUAL by default;
        {
            const vector<Document> expected_documents{{1, 0.866434, 5}, {0, 0.173287, 2}, {2, 0.173287, -1}};

            const auto& result_documents = server.FindTopDocuments("пушистый ухоженный кот"s);

            assert(result_documents.size() == expected_documents.size());

            for (int i = 0; i < result_documents.size(); i++) {
                const Document& rd = result_documents[i];
                const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
                assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
                assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
            }
            TRACE_DEBUG&& cout << "test: [" << TAG << " | ACTUAL by default with stop_words] completed successfully" << endl;
        }

        // BANNED
        {
            const vector<Document> expected_documents{{3, 0.231049, 9}};
            const auto& result_documents = server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);

            assert(result_documents.size() == expected_documents.size());

            for (int i = 0; i < result_documents.size(); i++) {
                const Document& rd = result_documents[i];
                const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
                assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
                assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
            }
            TRACE_DEBUG&& cout << "test: [" << TAG << " | BANNED with stop_words] completed successfully" << endl;
        }

        // Even ids
        {
            const vector<Document> expected_documents{{0, 0.173287, 2}, {2, 0.173287, -1}};
            const auto& result_documents = server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) {
                return document_id % 2 == 0;
            });

            assert(result_documents.size() == expected_documents.size());

            for (int i = 0; i < result_documents.size(); i++) {
                const Document& rd = result_documents[i];
                const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
                assert(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr));  // Not strict order matching
                assert(equalDocuments(rd, expected_documents[i]));         // Strict order matching
            }
            TRACE_DEBUG&& cout << "test: [" << TAG << " | Even ids with stop_words] completed successfully" << endl;
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | with stop_words] completed successfully" << endl;
    }
}

void TestSearchServer() {
    const bool TRACE_DEBUG = true;

    TRACE_DEBUG&& cout << "SearchServer testings." << endl;
    TRACE_DEBUG&& cout << "================================================================" << endl;
    TRACE_DEBUG&& cout << "+++ Start SearchServer testings..." << endl;

    int test_number = 0;

    ExpectTest(TestSplitWords, test_number, TRACE_DEBUG);

    ExpectTest(TestAddDocument, test_number, TRACE_DEBUG);

    ExpectTest(TestExcludeStopWordsFromAddedDocumentContent, test_number, TRACE_DEBUG);

    ExpectTest(TestStopWords, test_number, TRACE_DEBUG);

    ExpectTest(TestMinusWords, test_number, TRACE_DEBUG);

    ExpectTest(TestMatchDocuments, test_number, TRACE_DEBUG);

    ExpectTest(TestRelevanceSortOrder, test_number, TRACE_DEBUG);

    ExpectTest(TestRatingSortOrder, test_number, TRACE_DEBUG);

    ExpectTest(TestFilteringWihtPredicate, test_number, TRACE_DEBUG);

    ExpectTest(TestFindDocumentsBySpecifiedStatus, test_number, TRACE_DEBUG);

    ExpectTest(TestFindTopDocuments, test_number, TRACE_DEBUG);
    
    ExpectTest(TestCommon, test_number, TRACE_DEBUG);

    TRACE_DEBUG&& cout << endl << "+++ All SearchServer testings completed successfully." << endl;
    TRACE_DEBUG&& cout << "================================================================" << endl;
}
