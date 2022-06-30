#ifndef SERVER_HPP
#define SERVER_HPP

#include <functional>
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

struct Document {
    int id;
    double relevance;
    int rating;
    Document() : id{0}, relevance{0.0}, rating{0} {}
    Document(int id, double relevance, int rating) : id{id}, relevance{relevance}, rating{rating} {}
    friend ostream& operator<<(ostream& os, const Document& doc) {
        os << "{ document_id = " << doc.id << " relevance = " << doc.relevance << " rating = " << doc.rating << " }";
        return os;
    }
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

vector<string> SplitIntoWords(const string& text);

class SearchServer {
   public:
    SearchServer() = default;

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const string& stop_words_text);

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings);

    // template <typename T>
    vector<Document> FindTopDocuments(const string& raw_query, function<bool(int, DocumentStatus, int)> predicate) const;

    vector<Document> FindTopDocuments(const string& raw_query) const;

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const;

    int GetDocumentCount() const;

    int GetDocumentId(int index) const;

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const;

    set<std::string> GetStopWords() const;

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
    vector<int> documents_ids_queue_;

    bool IsStopWord(const string& word) const;

    vector<string> SplitIntoWordsNoStop(const string& text) const;

    static int ComputeAverageRating(const vector<int>& ratings);

    QueryWord ParseQueryWord(string text) const;

    Query ParseQuery(const string& text) const;

    double ComputeWordInverseDocumentFreq(const string& word) const;

    // template <typename T>
    vector<Document> FindAllDocuments(const Query& query, function<bool(int, DocumentStatus, int)> predicate) const;

    static bool IsValidWord(const string& word);

    template <typename List>
    static bool IsValidContent(const List& content);

    template <typename List>
    static bool IsValidStopWords(const List& stop_words);

    static bool IsValidMinusWords(const string& word);

    static bool IsValidMinusWords(const set<string>& words);

    bool IsValidDocumentId(int id) const;
};

class Page : protected vector<Document> {
   public:
    using super = vector<Document>;
    Page() : vector<Document>() {}
    explicit Page(int count) : vector<Document>(count) {}
    Page(int count, const Document& doc) : vector<Document>(count, doc) {}
    Page(iterator begin, iterator end) : vector<Document>(begin, end) {}

    friend ostream& operator<<(ostream& os, Page page) {
        for (auto ptr = page.begin(); ptr != page.end(); ptr++) {
            os << *ptr;
        }
        return os;
    }

    void AddDocument(const Document& document);

    // Container interface
    void Resize(size_t new_size);
    void Reserve(size_t n);
    bool IsEmpty() const;
    size_t Size() const;
    void Clear();

    // Iterator interface
    super::const_iterator begin() const;
    super::const_iterator end() const;
};

// template <template <typename...> class Container = vector, typename T = Page>
class Paginator : protected vector<Page> {
   public:
    using super = vector<Page>;
    Paginator(typename vector<Document>::const_iterator begin, typename vector<Document>::const_iterator end, size_t page_size);
    Paginator(typename set<Document>::const_iterator begin, typename set<Document>::const_iterator end, size_t page_size);
    
    template <class Iterator>
    static Paginator Create(Iterator begin, Iterator end, size_t page_size) {
        Paginator result(begin, end, page_size);
        return result;
    }
    
    void AddPage(Page page);

    // Iterator interface
    super::const_iterator begin() const;
    super::const_iterator end() const;

    size_t size() const;

   private:
    size_t page_size_;

    template <class Iterator>
    void inintialize(Iterator begin, Iterator end, size_t page_size);
};

#endif /* SERVER_HPP */
