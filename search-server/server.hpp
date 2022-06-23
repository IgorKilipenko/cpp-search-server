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

class SearchServer {
   public:
    void SetStopWords(const string& text);

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings);

    // template <typename T>
    vector<Document> FindTopDocuments(const string& raw_query, function<bool(int, DocumentStatus, int)> predicate) const;

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

    // template <typename T>
    vector<Document> FindAllDocuments(const Query& query, function<bool(int, DocumentStatus, int)> predicate) const;
};

#endif /* SERVER_HPP */
