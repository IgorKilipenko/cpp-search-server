#include "server_test.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

static const bool TRACE_DEBUG = true;

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

static string generateDocumentContent(int size, int length) {
    string words = ""s;
    srand(time(nullptr));

    while (size--) {
        int stringLen = (rand() % length) + 1;
        string s = "";
        for (int i = 0; i < stringLen; i++) {
            if (rand() % 2 == 0) {
                s += 'A' + (rand() % 26);
            } else {
                s += 'a' + (rand() % 26);
            }
        }
        words += s + " ";
    }
    if (!words.empty()) words.erase(words.end() - 1);
    return words;
}

static RawDocument generateRawDocument(int id, DocumentStatus status = DocumentStatus::ACTUAL) {
    string content = generateDocumentContent(5, 5);
    return {id, content, status, {0}};
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

static const vector<RawDocument> initial_documents{{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                                   {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                                   {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                                   {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}},
                                                   {4, ""s, DocumentStatus::ACTUAL, {0}}};

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

void TestSplitWords() {
    const string TAG = "TestSplitWords"s;
    const string stop_words = "   word   word1  word2    "s;
    const auto stops = SplitIntoWords(stop_words);
    assert(stops.size() == 3 && stops[0] == "word" && stops[2] == "word2");

    TRACE_DEBUG&& cout << "test: ["s << TAG << "] completed successfully" << endl;
}

void TestMinusWords() {
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
        // const set<string> minus_words = {"-ошейник", "-евгений"};
        int expected_count = 1;
        int expected_id = 2;
        const string query = "ухоженный белый -ошейник -евгений"s;
        const auto& top_documents = server.FindTopDocuments(query);
        assert(top_documents.size() == expected_count && top_documents[0].id == expected_id);
    }
    {
        const string query = "ухоженный белый кот пёс -ошейник -евгений"s;
        for (int i = 3; i <= 6; i++) {
            const auto& [words, _] = server.MatchDocument(query, i);
            assert(words.empty());
        }
        {
            const auto& [words, _] = server.MatchDocument(query, 0);
            assert(words.empty());
        }
        {
            const auto& [words, _] = server.MatchDocument(query, 2);
            assert(!words.empty());
        }
    }

    TRACE_DEBUG&& cout << "test: ["s << TAG << "] completed successfully" << endl;
}

void TestStopWords() {
    const string TAG = "TestStopWords"s;
    {
        const string stop_words = "word word1 word2"s;
        const set<string> stop_words_set = toSet(SplitIntoWords(stop_words));
        SearchServer server;
        server.SetStopWords(stop_words);
        vector<RawDocument> documents = {{0, "  белый кот и модный ошейник   "s, DocumentStatus::ACTUAL, {8, -3}},
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
 * @brief Добавление документов.
 * Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
 */
void TestAddDocument() {
    vector<RawDocument> documents = initial_documents;
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
        TRACE_DEBUG&& cout << "test: [TestMatchDocuments | Without minus_words] completed successfully" << endl;
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
        TRACE_DEBUG&& cout << "test: [TestMatchDocuments | With minus_words] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [TestMatchDocuments] completed successfully" << endl;
}

void TestFindTopDocuments() {
    const string TAG = "TestFindTopDocuments"s;
    // Test on know documents from the example (without stop_words)
    {
        /*
         * WITHOUT STOP WORDS:
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

        // WITHOUT STOP WORDS:
        auto test_without_stop_words = [&TAG](void) {
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
                const auto& result_documents =
                    server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) {
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
        };
        test_without_stop_words();

        // WITHOUT STOP WORDS:
        auto test_with_stop_words = [&TAG](void) {
            /*
             * WITH STOP WORDS:
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
                const auto& result_documents =
                    server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) {
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
        };
        test_with_stop_words();
    }

    // Max top count (sort by rating) documents test
    {
        const double threshold = 1e-6;
        const int documents_size = 100;
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

        const auto& matching_documents = server.FindTopDocuments(word + " ft");
        assert(matching_documents.size() == expected_top_documents_count);

        vector<RawDocument> expected_documents(raw_documents.end() - expected_top_documents_count, raw_documents.end());
        reverse(expected_documents.begin(), expected_documents.end());

        for (int i = 0; i < expected_top_documents_count; i++) {
            assert(expected_documents[i].id == matching_documents[i].id);
        }
        for (int i = 0; i < expected_top_documents_count - 1; i++) {
            const double relevance_order = matching_documents[i].relevance - matching_documents[i + 1].relevance;
            assert(relevance_order > 0 || (abs(relevance_order) < threshold && matching_documents[i].rating >= matching_documents[i + 1].rating));
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | Max top count (sort by rating)] completed successfully" << endl;
    }

    // Max top count (full sort order) documents test
    {
        const double threshold = 1e-6;
        const int documents_size = 1000;
        const int expected_top_documents_count = MAX_RESULT_DOCUMENT_COUNT;

        SearchServer server;

        const string word = "word";
        vector<RawDocument> raw_documents;
        raw_documents.reserve(documents_size);

        for (int i = 0; i < documents_size; i++) {
            const bool condition = i > max(min(documents_size / 2, documents_size - MAX_RESULT_DOCUMENT_COUNT - 1), 0);
            string content = word + (condition ? "" : "#" + to_string(i));
            for (int j = 0; j < min(i, 20); j++) {
                if (!condition) {
                    content += " " + ((i + j) % 2 ? word : word + to_string(j + i));
                } else {
                    content += " " + word;
                }
            }
            const RawDocument doc = {i, content, DocumentStatus::ACTUAL, {i, documents_size}};
            raw_documents.push_back(doc);
            server.AddDocument(doc.id, doc.content, doc.status, doc.ratings);
        }

        const auto& matching_documents = server.FindTopDocuments(word + " ft");
        assert(matching_documents.size() == expected_top_documents_count);

        vector<RawDocument> expected_documents(raw_documents.end() - expected_top_documents_count, raw_documents.end());
        reverse(expected_documents.begin(), expected_documents.end());

        for (int i = 0; i < expected_top_documents_count; i++) {
            assert(expected_documents[i].id == matching_documents[i].id);
        }
        for (int i = 0; i < expected_top_documents_count - 1; i++) {
            const double relevance_order = matching_documents[i].relevance - matching_documents[i + 1].relevance;
            assert(relevance_order > 0 || (abs(relevance_order) < threshold && matching_documents[i].rating >= matching_documents[i + 1].rating));
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | Max top count (full sort order)] completed successfully" << endl;
    }
}

template <typename T>
void ExpectTest(T (*test)(void), int& test_number) {
    TRACE_DEBUG&& cout << endl << "Starting [test #"s << ++test_number << "]..." << endl;
    test();
    TRACE_DEBUG&& cout << "[Test #"s << test_number << "] completed successfully."s << endl;
}

void TestSearchServer() {
    TRACE_DEBUG&& cout << "SearchServer testings." << endl;
    TRACE_DEBUG&& cout << "================================================================" << endl;
    TRACE_DEBUG&& cout << "+++ Start SearchServer testings..." << endl;

    int test_number = 0;

    ExpectTest(TestSplitWords, test_number);

    ExpectTest(TestAddDocument, test_number);

    ExpectTest(TestExcludeStopWordsFromAddedDocumentContent, test_number);

    ExpectTest(TestStopWords, test_number);

    ExpectTest(TestMinusWords, test_number);

    ExpectTest(TestMatchDocuments, test_number);

    ExpectTest(TestFindTopDocuments, test_number);

    TRACE_DEBUG&& cout << endl << "+++ All SearchServer testings completed successfully." << endl;
    TRACE_DEBUG&& cout << "================================================================" << endl;
}
