#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <vector>

using namespace std;

static const double THRESHOLD = 1e-6;

#if defined(SERVER_TEST) && SERVER_TEST

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
   public:
    void SetStopWords(const string& text);

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings);

    template <typename T>
    vector<Document> FindTopDocuments(const string& raw_query, T predicate) const;

    vector<Document> FindTopDocuments(const string& raw_query) const;

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const;

    int GetDocumentCount() const;

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const;

   private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const;

    vector<string> SplitIntoWordsNoStop(const string& text) const;

    static int ComputeAverageRating(const vector<int>& ratings);

    QueryWord ParseQueryWord(string text) const;

    Query ParseQuery(const string& text) const;

    double ComputeWordInverseDocumentFreq(const string& word) const;

    template <typename T>
    vector<Document> FindAllDocuments(const Query& query, T predicate) const;
};

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

void SearchServer::SetStopWords(const string& text) {
    for (const string& word : SplitIntoWords(text)) {
        stop_words_.insert(word);
    }
}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
}

template <typename T>
vector<Document> SearchServer::FindTopDocuments(const string& raw_query, T predicate) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, predicate);

    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        if (abs(lhs.relevance - rhs.relevance) < THRESHOLD) {
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        }
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, [](int id, DocumentStatus status, [[maybe_unused]] int rating) -> bool {
        return status == DocumentStatus::ACTUAL;
    });
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int id, DocumentStatus doc_status, [[maybe_unused]] int rating) -> bool {
        return (doc_status == status);
    });
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }

    const int rating_sum = accumulate(ratings.begin(), ratings.end(), 0, [](int summ, int rating) {
        return summ + rating;
    });

    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query query;
    for (const string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

template <typename T>
vector<Document> SearchServer::FindAllDocuments(const Query& query, T predicate) const {
    map<int, double> document_to_relevance;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            if (!documents_.count(document_id)) continue;
            const auto& document = documents_.at(document_id);
            if (predicate(document_id, document.status, document.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

#endif  // SERVER_TEST

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

/*
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
*/

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
void TestRatingSortOrder() {
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

    const string query = shared_word;
    // Strict requests
    {// Test ACTUAL
     {const int request_id = 0;
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

            const auto& matched_documents = server.FindTopDocuments(query, []([[maybe_unused]] int id, DocumentStatus status, int rating) -> bool {
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
}
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
}

void TestFindTopDocuments() {
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
    }
}

void TestSearchServer() {
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
}

int main() {
    TestSearchServer();
    cout << "Search server testing finished"s << endl;

    return 0;
}
