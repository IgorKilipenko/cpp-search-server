#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstddef>
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

template <typename TDocument = Document>
class Page : protected vector<TDocument> {
   public:
    using super = vector<TDocument>;
    using InIterator = typename Page::const_iterator;

    Page() : vector<TDocument>() {}

    explicit Page(int count) : vector<TDocument>(count) {}

    Page(int count, const TDocument& doc) : vector<TDocument>(count, doc) {}

    Page(InIterator begin, InIterator end) : vector<TDocument>(begin, end) {}

    friend ostream& operator<<(ostream& os, Page page) {
        for (auto ptr = page.begin(); ptr != page.end(); ptr++) {
            os << *ptr;
        }
        return os;
    }

    void AddDocument(const TDocument& document);

    // Container interface
    void Resize(size_t new_size);
    void Reserve(size_t n);
    bool IsEmpty() const;
    size_t Size() const;
    void Clear();

    // Iterator interface
    auto begin() const;
    auto end() const;
};

template <class TPage = Page<Document>>
class Paginator : protected vector<TPage> {
   public:
    using super = vector<TPage>;
    using DocIterator = typename TPage::InIterator;

    explicit Paginator(size_t page_size) : page_size_{page_size} {}
    Paginator(DocIterator begin, DocIterator end, size_t page_size);

    /// Add new page
    void AddPage(TPage page);

    /// Get page by index.
    /// This function provides for safer data access.
    TPage& GetPage(size_t index);

    /// Get page by index.
    /// This function provides for safer data access.
    const TPage& GetPage(size_t index) const;

    /// Get page by index
    TPage& operator[](size_t index) {
        return super::operator[](index);
    }

    /// First iteration element
    auto begin() const;

    /// Last iteration element
    auto end() const;

    /// Total page count
    size_t size() const;

    /// Get size of page
    size_t PageSize() const;

   private:
    size_t page_size_;
};

template <class TPage>
Paginator<TPage>::Paginator(DocIterator begin, DocIterator end, size_t page_size) : Paginator(page_size) {
    TPage curr_page{};
    curr_page.Reserve(page_size);
    int i = 0;
    for (auto ptr = begin; ptr != end; ptr++) {
        curr_page.AddDocument(*ptr);
        if (++i > 0 && i % page_size_ == 0) {
            this->AddPage(curr_page);
            curr_page.Clear();
        }
    }
    if (!curr_page.IsEmpty()) {
        curr_page.Resize(curr_page.Size());
        this->AddPage(curr_page);
    }
}

template <class TPage>
void Paginator<TPage>::AddPage(TPage page) {
    this->push_back(page);
}

template <class TPage>
TPage& Paginator<TPage>::GetPage(size_t index) {
    return super::at(index);
}

template <class TPage>
const TPage& Paginator<TPage>::GetPage(size_t index) const {
    return super::at(index);
}

template <class TPage>
auto Paginator<TPage>::begin() const {
    return super::begin();
}

template <class TPage>
auto Paginator<TPage>::end() const {
    return super::end();
}

template <class TPage>
size_t Paginator<TPage>::size() const {
    return super::size();
}

template <typename TDocument>
void Page<TDocument>::AddDocument(const TDocument& document) {
    super::push_back(document);
}

template <typename TDocument>
void Page<TDocument>::Resize(size_t new_size) {
    super::resize(new_size);
}

template <typename TDocument>
void Page<TDocument>::Reserve(size_t n) {
    super::reserve(n);
}

template <typename TDocument>
bool Page<TDocument>::IsEmpty() const {
    return super::empty();
}

template <typename TDocument>
size_t Page<TDocument>::Size() const {
    return super::size();
}

template <typename TDocument>
void Page<TDocument>::Clear() {
    super::clear();
}

template <typename TDocument>
auto Page<TDocument>::begin() const {
    return super::begin();
}

template <typename TDocument>
auto Page<TDocument>::end() const {
    return super::end();
}

#endif /* SERVER_HPP */
