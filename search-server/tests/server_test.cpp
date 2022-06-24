#include "server_test.hpp"

#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

static const bool TRACE_DEBUG = true;

/* OSTREAM operator<< overrides region -------------------------------------- */

template <typename T>
ostream& _printCollection(ostream& os, const T& values) {
    string sep = "";
    for (const auto& v : values) {
        os << sep << v;
        if (sep.empty()) sep = ", ";
    }
    return os;
}

template <typename T>
ostream& _printDictionary(ostream& os, const T& values) {
    string sep = "";
    for (const auto& [k, v] : values) {
        os << sep << k << ": " << v;
        if (sep.empty()) sep = ", ";
    }
    return os;
}

template <typename T>
ostream& operator<<(ostream& os, const vector<T> values) {
    os << "[";
    _printCollection(os, values);
    os << "]";
    return os;
}

template <typename T>
ostream& operator<<(ostream& os, const set<T> values) {
    os << "{";
    _printCollection(os, values);
    os << "}";
    return os;
}

template <typename K, typename V>
ostream& operator<<(ostream& os, const map<K, V> values) {
    os << "{";
    _printDictionary(os, values);
    os << "}";
    return os;
}

ostream& operator<<(ostream& os, DocumentStatus status) {
    switch (status) {
        case DocumentStatus::ACTUAL:
            os << "ACTUAL"s;
            break;
        case DocumentStatus::IRRELEVANT:
            os << "IRRELEVANT"s;
            break;
        case DocumentStatus::BANNED:
            os << "BANNED"s;
            break;
        case DocumentStatus::REMOVED:
            os << "REMOVED"s;
            break;
    }
    return os;
}

/* ASSERTION define region -------------------------------------- */

#if !defined(RUN_TEST)

template <typename T>
void RunTestImpl(T func, const string& func_name) {
    func();
    cerr << func_name << " success" << endl;
}

#define RUN_TEST(func) RunTestImpl((func), (#func))

#endif  // !RUN_TEST

#if !defined(ASSERT)

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file, const string& func, unsigned line,
                     const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line, const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#endif  // !ASSERT

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

static void testStrictOrderEqual(vector<Document> result_documents, vector<Document> expected_documents) {
    ASSERT_EQUAL(result_documents.size(), expected_documents.size());
    for (int i = 0; i < result_documents.size(); i++) {
        const Document& rd = result_documents[i];
        const shared_ptr<Document> ed_ptr = getDocumentById(rd.id, expected_documents);
        ASSERT_HINT(ed_ptr != nullptr && equalDocuments(rd, *ed_ptr), "Non-strict order does not match."s);
        ASSERT_HINT(equalDocuments(rd, expected_documents[i]), "Non-strict order does not match."s);
    }
}

/* ---------------------------------------------------------------- */

static const vector<RawDocument> initial_documents{{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                                   {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                                   {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                                   {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}},
                                                   {4, ""s, DocumentStatus::ACTUAL, {0}}};

void TestSplitWords() {
    const string TAG = "TestSplitWords"s;
    const string stop_words = "   word   word1  word2    "s;
    const auto stops = SplitIntoWords(stop_words);
    ASSERT_EQUAL(stops.size(), 3);
    ASSERT_EQUAL(stops[0], "word");
    ASSERT_EQUAL(stops[2], "word2");

    TRACE_DEBUG&& cerr << "test: ["s << TAG << "] completed successfully" << endl;
}

/**
 * @brief Поддержка стоп-слов.
 * Стоп-слова исключаются из текста документов.
 * @param TRACE_DEBUG
 */
void TestStopWords() {
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
                                         {6, "ухоженный скворец евгений word word1 word2"s, DocumentStatus::ACTUAL, {9}},
                                         {7, stop_words, DocumentStatus::ACTUAL, {0}},
                                         {8, ""s, DocumentStatus::ACTUAL, {0}}};

        for (const auto& [id, content, status, ratings] : documents) {
            server.AddDocument(id, content, status, ratings);
        }

        {
            const auto found_docs = server.FindTopDocuments(" "s);
            ASSERT_EQUAL(found_docs.size(), 0);
        }

        {
            const vector<string> queries{"евгений word", "глаза", "ухоженный"};
            for (auto& query : queries) {
                const auto found_docs = server.FindTopDocuments(query);
                ASSERT_EQUAL(found_docs.size(), (query == queries[0] ? 4 : query == queries[1] ? 1 : 5));
                for (const auto& doc : found_docs) {
                    const auto& [words, status] = server.MatchDocument(query, doc.id);
                    ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
                    for (const auto& w : words) {
                        ASSERT(!stop_words_set.count(w));
                    }
                }
            }
        }

        // Loop for all stop words
        {
            for (const auto& sw : stop_words_set) {
                const auto& found_docs = server.FindTopDocuments(sw);
                for (const auto& doc : found_docs) {
                    const auto& [words, status] = server.MatchDocument(sw, doc.id);
                    ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
                    for (const auto& w : words) {
                        ASSERT(!stop_words_set.count(w));
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
        int expected_count = 1;
        int expected_id = 2;
        const string query = "ухоженный белый -ошейник -евгений"s;
        const auto& top_documents = server.FindTopDocuments(query);
        ASSERT_EQUAL(top_documents.size(), expected_count);
        ASSERT_EQUAL(top_documents[0].id, expected_id);
        ASSERT(!count_if(top_documents.begin(), top_documents.end(), [](const auto& doc) {
            return doc.id == 0 || doc.id > 2;
        }));
    }

    {
        const string query = "ухоженный белый кот пёс -ошейник -евгений"s;
        {
            const auto& [words, _] = server.MatchDocument(query, 0);
            ASSERT(words.empty());
        }
        {
            const auto& [words, _] = server.MatchDocument(query, 2);
            ASSERT(!words.empty());
        }
        for (int i = 3; i <= 6; i++) {
            const auto& [words, _] = server.MatchDocument(query, i);
            ASSERT(words.empty());
        }
    }

    TRACE_DEBUG&& cout << "test: ["s << TAG << "] completed successfully" << endl;
}

/**
 * @brief Добавление документов.
 * Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
 */
void TestAddDocument() {
    const string TAG = "TestAddDocument";
    vector<RawDocument> documents = initial_documents;
    SearchServer server;

    int expected_count = 0;
    ASSERT_EQUAL(server.GetDocumentCount(), expected_count);

    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
        ASSERT_EQUAL(server.GetDocumentCount(), ++expected_count);
    }

    // Test for document is matching
    {
        for (const auto& [id, content, status, ratings] : documents) {
            const auto& [words, mathed_status] = server.MatchDocument(content, id);
            ASSERT_EQUAL(mathed_status, status);
            ASSERT(!words.empty() || (content.empty() && words.empty()));
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
            ASSERT(tops_count || (raw_doc.content.empty() && tops_count == 0));
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
void TestMatchDocuments() {
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

            ASSERT_EQUAL(status, doc.status);  // Check status

            auto query_words = SplitIntoWords(raw_query);
            const int mathed_words_count = count_if(matched_words.begin(), matched_words.end(), [&query_words](const auto& mw) {
                int _count = count(query_words.begin(), query_words.end(), mw);
                ASSERT(_count > 0);  // Mathed word must contained in query
                return _count ? 1 : 0;
            });

            auto doc_words = SplitIntoWords(doc.content);
            const int expected_mathed_words_count = count_if(query_words.begin(), query_words.end(), [&doc_words](const auto& qw) {
                int _count = count(doc_words.begin(), doc_words.end(), qw);
                return _count ? 1 : 0;
            });

            ASSERT_EQUAL(expected_mathed_words_count, mathed_words_count);
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
        ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
        ASSERT(matched_words.empty());

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

            ASSERT_EQUAL(status, doc.status);  // Check status
            ASSERT_HINT(!count(matched_words.begin(), matched_words.end(), expected_minus_word), "Matched_words contained minus_words");

            auto query_words = SplitIntoWords(raw_query);
            query_words.erase(query_words.end() - 1);  // Remove minus_word

            const int mathed_words_count = count_if(matched_words.begin(), matched_words.end(), [&query_words](const auto& mw) {
                int _count = count(query_words.begin(), query_words.end(), mw);
                ASSERT_HINT(_count > 0, "Mathed word not contained in query."s);
                return _count ? 1 : 0;
            });

            auto doc_words = SplitIntoWords(doc.content);
            bool is_contained_minus_words = count(doc_words.begin(), doc_words.end(), expected_minus_word);
            const int expected_mathed_words_count =
                is_contained_minus_words ? 0 : count_if(query_words.begin(), query_words.end(), [&doc_words](const auto& qw) {
                    int _count = count(doc_words.begin(), doc_words.end(), qw);
                    return _count ? 1 : 0;
                });

            ASSERT_EQUAL(expected_mathed_words_count, mathed_words_count);
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
void TestRelevanceSortOrder() {
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

        testStrictOrderEqual(result_documents, expected_documents);

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

        testStrictOrderEqual(result_documents, expected_documents);

        TRACE_DEBUG&& cout << "test: [" << TAG << " | with stop_words] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Сортировка найденных документов по релевантности.
 * Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
 * @param TRACE_DEBUG
 */
void TestRatingSortOrder() {
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
    ASSERT(matching_documents.size() >= expected_top_documents.size());

    for (int i = 0; i < expected_top_documents.size(); i++) {
        ASSERT_EQUAL(matching_documents[i].id, expected_top_documents[i].id);
        ASSERT_EQUAL(matching_documents[i].rating, expected_top_documents[i].rating);
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Фильтрация результатов поиска.
 * Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
 * @param TRACE_DEBUG
 */
void TestFilteringWihtPredicate() {
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
    {
        // Test ACTUAL
        {
            const int request_id = 0;
            const auto request_status = DocumentStatus::ACTUAL;
            const int request_rating = 2;

            const auto& matched_documents = server.FindTopDocuments(query, [](int id, DocumentStatus status, int rating) -> bool {
                return id == request_id && status == request_status && rating == request_rating;
            });
            ASSERT_EQUAL(matched_documents.size(), 1);

            const auto& matched_doc = matched_documents.front();
            ASSERT_EQUAL(matched_doc.id, request_id);
            ASSERT_EQUAL(matched_doc.rating, request_rating);
        }

        // Test REMOVED
        {
            const int request_id = 3;
            const auto request_status = DocumentStatus::REMOVED;
            const int request_rating = 9;

            const auto& matched_documents = server.FindTopDocuments(query, [](int id, DocumentStatus status, int rating) -> bool {
                return id == request_id && status == request_status && rating == request_rating;
            });
            ASSERT_EQUAL(matched_documents.size(), 1);

            const auto& matched_doc = matched_documents.front();
            ASSERT_EQUAL(matched_doc.id, request_id);
            ASSERT_EQUAL(matched_doc.rating, request_rating);
        }
        TRACE_DEBUG&& cout << "test: [" << TAG << " | Strict requests] completed successfully" << endl;
    }

    // Not Strict requests
    {
        // Test ACTUAL
        {
            // Expect success
            {
                const auto request_status = DocumentStatus::ACTUAL;
                const int request_min_rating = 1;
                const int expected_id = 0;
                const int expected_rating = 2;

                const auto& matched_documents =
                    server.FindTopDocuments(query, []([[maybe_unused]] int id, DocumentStatus status, int rating) -> bool {
                        return status == request_status && rating > request_min_rating;
                    });
                ASSERT_EQUAL(matched_documents.size(), 1);

                const auto& matched_doc = matched_documents.front();
                ASSERT_EQUAL(matched_doc.id, expected_id);
                ASSERT_EQUAL(matched_doc.rating, expected_rating);
            }

            // Expect failure
            {
                {
                    const auto request_status = DocumentStatus::ACTUAL;
                    const int request_min_rating = 3;

                    const auto& matched_documents =
                        server.FindTopDocuments(query, []([[maybe_unused]] int id, DocumentStatus status, int rating) -> bool {
                            return status == request_status && rating > request_min_rating;
                        });
                    ASSERT_EQUAL(matched_documents.size(), 0);
                }
                {
                    const auto request_status = DocumentStatus::REMOVED;
                    const int request_id = 0;

                    const auto& matched_documents =
                        server.FindTopDocuments(query, [](int id, DocumentStatus status, [[maybe_unused]] int rating) -> bool {
                            return status == request_status && id == request_id;
                        });
                    ASSERT_EQUAL(matched_documents.size(), 0);
                }
            }
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | Not Strict requests] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

/**
 * @brief Поиск документов, имеющих заданный статус.
 *
 * @param TRACE_DEBUG
 */
void TestFindDocumentsBySpecifiedStatus() {
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
        ASSERT_EQUAL(matched_documents.size(), 1);
        ASSERT_EQUAL(matched_documents.front().id, raw_doc.id);
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

void TestFindTopDocuments() {
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
        ASSERT_EQUAL(matched_documents.size(), expected_top_documents_count);

        vector<RawDocument> expected_documents(raw_documents.end() - expected_top_documents_count, raw_documents.end());
        reverse(expected_documents.begin(), expected_documents.end());

        for (int i = 0; i < expected_top_documents_count; i++) {
            ASSERT_EQUAL(expected_documents[i].id, matched_documents[i].id);
        }
        for (int i = 0; i < expected_top_documents_count - 1; i++) {
            const double relevance_order = matched_documents[i].relevance - matched_documents[i + 1].relevance;
            ASSERT(relevance_order > 0 || (abs(relevance_order) < threshold && matched_documents[i].rating >= matched_documents[i + 1].rating));
        }

        TRACE_DEBUG&& cout << "test: [" << TAG << " | Max top count (sort by rating)] completed successfully" << endl;
    }

    TRACE_DEBUG&& cout << "test: [" << TAG << "] completed successfully" << endl;
}

void TestSearchServer() {
    TRACE_DEBUG&& cout << "SearchServer testings." << endl;
    TRACE_DEBUG&& cout << "================================================================" << endl;
    TRACE_DEBUG&& cout << "+++ Start SearchServer testings..." << endl;

    RUN_TEST(TestSplitWords);

    RUN_TEST(TestAddDocument);

    RUN_TEST(TestStopWords);

    RUN_TEST(TestMinusWords);

    RUN_TEST(TestMatchDocuments);

    RUN_TEST(TestRelevanceSortOrder);

    RUN_TEST(TestRatingSortOrder);

    RUN_TEST(TestFilteringWihtPredicate);

    RUN_TEST(TestFindDocumentsBySpecifiedStatus);

    RUN_TEST(TestFindTopDocuments);

    TRACE_DEBUG&& cout << endl << "+++ All SearchServer testings completed successfully." << endl;
    TRACE_DEBUG&& cout << "================================================================" << endl;
}
