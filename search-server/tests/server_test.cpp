#include "server_test.hpp"

#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <vector>

static const double THRESHOLD = 1e-6;

/* OSTREAM operator<< overrides region -------------------------------------- */

template <typename T>
ostream& printCollection(ostream& os, const T& values) {
    string sep = "";
    for (const auto& v : values) {
        os << sep << v;
        if (sep.empty()) sep = ", ";
    }
    return os;
}

template <typename T>
ostream& printDictionary(ostream& os, const T& values) {
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
    printCollection(os, values);
    os << "]";
    return os;
}

template <typename T>
ostream& operator<<(ostream& os, const set<T> values) {
    os << "{";
    printCollection(os, values);
    os << "}";
    return os;
}

template <typename K, typename V>
ostream& operator<<(ostream& os, const map<K, V> values) {
    os << "{";
    printDictionary(os, values);
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

template <typename T>
static set<T> toSet(const vector<T>& values) {
    set<T> result(values.begin(), values.end());
    return result;
}

/* ---------------------------------------------------------------- */

static const vector<RawDocument> initial_documents{{0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
                                                   {1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7}},
                                                   {2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1}},
                                                   {3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9}},
                                                   {4, ""s, DocumentStatus::ACTUAL, {0}}};

void TestSplitWords() {
    const string stop_words = "   word   word1  word2    "s;
    const auto stops = SplitIntoWords(stop_words);
    ASSERT_EQUAL(stops.size(), 3);
    ASSERT_EQUAL(stops[0], "word");
    ASSERT_EQUAL(stops[2], "word2");
}

/**
 * @brief Поддержка стоп-слов.
 * Стоп-слова исключаются из текста документов.
 */
void TestStopWords() {
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
            const map<string, int> expected_mathed_docs_count = {{queries[0], 4}, {queries[1], 1}, {queries[2], 5}};
            for (auto& query : queries) {
                const auto found_docs = server.FindTopDocuments(query);
                ASSERT_EQUAL(found_docs.size(), expected_mathed_docs_count.at(query));
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
        }
    }
}

/**
 * @brief Поддержка минус-слов.
 * Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
 */
void TestMinusWords() {
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
}

/**
 * @brief Добавление документов.
 * Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
 */
void TestAddDocument() {
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
    }
}

/**
 * @brief Матчинг документов.
 * При матчинге документа по поисковому запросу должны быть возвращены все слова из поискового запроса, присутствующие в документе.
 * Если есть соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
 */
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

            ASSERT_EQUAL_HINT(status, doc.status, "Document status is not as expected."s);

            auto query_words = SplitIntoWords(raw_query);
            const int mathed_words_count = count_if(matched_words.begin(), matched_words.end(), [&query_words](const auto& mw) {
                int _count = count(query_words.begin(), query_words.end(), mw);
                ASSERT_HINT(_count > 0, "Word \""s + mw + "\" not contained in query."s);
                return _count ? 1 : 0;
            });

            auto doc_words = SplitIntoWords(doc.content);
            const int expected_mathed_words_count = count_if(query_words.begin(), query_words.end(), [&doc_words](const auto& qw) {
                int _count = count(doc_words.begin(), doc_words.end(), qw);
                return _count ? 1 : 0;
            });

            ASSERT_EQUAL(expected_mathed_words_count, mathed_words_count);
        }
    }

    // With minus_words an stop words test
    {
        const string initial_content = "белый кот и модный ошейник"s;
        RawDocument raw_doc = {0, initial_content, DocumentStatus::ACTUAL, {8, -3}};
        vector<RawDocument> raw_documents = {raw_doc};
        string minus_word = "кот"s;
        string stop_word = "белый"s;

        SearchServer server;
        server.SetStopWords(stop_word);

        for (const auto& [id, content, status, ratings] : raw_documents) {
            server.AddDocument(id, content, status, ratings);
        }

        // Empty content check (for minus_words)
        {
            const string query = initial_content + " -" + minus_word;
            const auto& [matched_words, _] = server.MatchDocument(query, raw_doc.id);
            ASSERT_HINT(matched_words.empty(), "Document content for the minus_words query is not empty.");
        }

        // Matching does not contain minus_words
        {
            const string query = initial_content;
            const auto& [matched_words, _] = server.MatchDocument(query, raw_doc.id);
            ASSERT_HINT(!matched_words.empty() && none_of(matched_words.begin(), matched_words.end(),
                                                          [stop_word](const string& word) {
                                                              return word == stop_word;
                                                          }),
                        "Document contains stop_word:"s + stop_word + "."s);
        }
    }
}

/**
 * @brief Сортировка найденных документов по релевантности.
 * Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
 */
void TestRelevanceSortOrder() {
    const DocumentStatus status = DocumentStatus::ACTUAL;
    // Documents from example
    vector<RawDocument> documents = {{0, "белый кот и модный ошейник"s, status, {8, -3}},
                                     {1, "пушистый кот пушистый хвост"s, status, {7, 2, 7}},
                                     {2, "ухоженный пёс выразительные глаза"s, status, {5, -12, 2, 1}},
                                     {3, "ухоженный скворец евгений"s, status, {9}}};

    const string query = "пушистый ухоженный кот"s;
    const int expected_top_documents_count = min(documents.size(), static_cast<size_t>(MAX_RESULT_DOCUMENT_COUNT));

    SearchServer server;
    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
    }

    const auto& result_documents = server.FindTopDocuments(query, status);

    ASSERT_EQUAL(result_documents.size(), expected_top_documents_count);

    for (int i = 0; i < result_documents.size() - 1; i++) {
        const auto& curr_doc = result_documents[i];
        const auto& next_doc = result_documents[i + 1];
        ASSERT(curr_doc.relevance > next_doc.relevance ||
               (abs(curr_doc.relevance - next_doc.relevance) < THRESHOLD && curr_doc.rating >= next_doc.rating));
    }
}

/**
 * @brief Вычисление рейтинга документов.
 * Рейтинг добавленного документа равен среднему арифметическому оценок документа.
 */
void TestRatingCulculation() {
    const auto culcAvgRating = [](const vector<int>& ratings) -> int {
        if (ratings.empty()) return 0;
        const double n = static_cast<double>(ratings.size());
        const double avg = accumulate(ratings.begin(), ratings.end(), 0.0, [n](double curr, int v) {
            curr += v / n;
            return curr;
        });
        return static_cast<int>(avg);
    };

    const DocumentStatus status = DocumentStatus::ACTUAL;
    const string content = "word1 word2 word3"s;
    const string query = content;
    // Raw documents
    vector<RawDocument> documents = {{0, content, status, {8, -3}},
                                     {1, content, status, {7, 2, 7}},
                                     {2, content, status, {5, -12, 2, 1}},
                                     {3, content, status, {9}},
                                     {4, content, status, {19, 2}}};

    SearchServer server;
    for (const auto& [id, content, status, ratings] : documents) {
        server.AddDocument(id, content, status, ratings);
    }

    map<int, int> expected_rating_values{};
    for (const auto& raw_doc : documents) {
        expected_rating_values.insert({raw_doc.id, culcAvgRating(raw_doc.ratings)});
    }

    const auto& matching_documents = server.FindTopDocuments(query);
    ASSERT_HINT(matching_documents.size(), "No matching documents found."s);

    for (const auto& doc : matching_documents) {
        ASSERT_EQUAL(doc.rating, expected_rating_values[doc.id]);
    }
}

/**
 * @brief Фильтрация результатов поиска.
 * Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
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

    const int request_id = 2;
    const string query = shared_word;
    const auto request_status = DocumentStatus::IRRELEVANT;

    const auto& matched_documents = server.FindTopDocuments(query, [](int id, DocumentStatus status, int rating) -> bool {
        return id == request_id && status == request_status && rating < 0;
    });

    ASSERT_HINT(matched_documents.size(), "No matching documents found."s);

    const auto& matched_doc = matched_documents.front();
    ASSERT_EQUAL(matched_doc.id, request_id);
}

/**
 * @brief Поиск документов, имеющих заданный статус.
 *
 */
void TestFindDocumentsBySpecifiedStatus() {
    const string shared_word = "word"s;
    const string query = shared_word;
    vector<RawDocument> documents = {
        {static_cast<int>(DocumentStatus::ACTUAL), shared_word + " белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3}},
        {static_cast<int>(DocumentStatus::BANNED), shared_word + " пушистый кот пушистый хвост"s, DocumentStatus::BANNED, {7, 2, 7}},
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

    // Expect search failure (empty result)
    const auto& matched_documents = server.FindTopDocuments(query, DocumentStatus::IRRELEVANT);
    ASSERT(matched_documents.empty());
}

void TestFindTopDocuments() {
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
}

void TestSearchServer() {
    RUN_TEST(TestSplitWords);

    RUN_TEST(TestAddDocument);

    RUN_TEST(TestStopWords);

    RUN_TEST(TestMinusWords);

    RUN_TEST(TestMatchDocuments);

    RUN_TEST(TestRelevanceSortOrder);

    RUN_TEST(TestRatingCulculation);

    RUN_TEST(TestFilteringWihtPredicate);

    RUN_TEST(TestFindDocumentsBySpecifiedStatus);

    RUN_TEST(TestFindTopDocuments);
}
