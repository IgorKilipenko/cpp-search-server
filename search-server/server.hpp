#ifndef SERVER_HPP
#define SERVER_HPP

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

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
    explicit SearchServer(const StringContainer& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {}

    explicit SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
    {}

    [[nodiscard]] bool AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings);

    // template <typename T>
    [[nodiscard]] bool FindTopDocuments(const string& raw_query, function<bool(int, DocumentStatus, int)> predicate, vector<Document>& result) const;

    [[nodiscard]] bool FindTopDocuments(const string& raw_query, vector<Document>& result) const;

    [[nodiscard]] bool FindTopDocuments(const string& raw_query, DocumentStatus status, vector<Document>& result) const;

    int GetDocumentCount() const;

    int GetDocumentId(int index) const;

    [[nodiscard]] bool MatchDocument(const string& raw_query, int document_id, tuple<vector<string>, DocumentStatus>& result) const;

    set<std::string> GetStopWords() const;

    inline static constexpr int INVALID_DOCUMENT_ID = -1;
    
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

    // template <typename T>
    vector<Document> FindAllDocuments(const Query& query, function<bool(int, DocumentStatus, int)> predicate) const;

    static bool IsValidWord(const string& word);

    template <typename List>
    static bool IsValidContent(const List& content);

    static bool IsValidMinusWords(const string& word);

    static bool IsValidMinusWords(const set<string>& words);

    bool IsValidDocumentId(int id) const;
};

#endif /* SERVER_HPP */
