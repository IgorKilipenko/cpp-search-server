#ifndef SERVER_HPP
#define SERVER_HPP

#ifndef EXPORT
#define EXPORT
#endif

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using DocumentIndex = pair<string, set<int>>;
using DocumentsIndexTable = map<string, set<int>>;

struct Query {
    set<string> plus_words;
    set<string> minus_words;
};

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus { ACTUAL, IRRELEVANT, BANNED, REMOVED };

class SearchServer {
   public:
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    struct DocumentInfo {
        int rating;
        DocumentStatus status;
    };
    
    void SetStopWords(const string& text);
    void AddDocument(int document_id, const string& document,
                     DocumentStatus status, const vector<int>& ratings);
    vector<Document> FindTopDocuments(
        const string& raw_query,
        DocumentStatus status = DocumentStatus::ACTUAL) const;
    static vector<int> ReadRatingsLine();

   private:
    int document_count_ = 0;
    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentInfo> document_info_;

    bool IsStopWord(const string& word) const;
    vector<string> SplitIntoWordsNoStop(const string& text) const;
    QueryWord ParseQueryWord(string text) const;
    Query ParseQuery(const string& text) const;
    vector<Document> FindAllDocuments(const Query& query,
                                      DocumentStatus status) const;
    map<int, double> MatchDocument(const DocumentsIndexTable& doc_ids_table,
                                   const Query& query) const;
    double ComputeWordInverseDocumentFreq(const string& word) const;
    static int ComputeAverageRating(const vector<int>& ratings);
};

EXPORT int exec_main();

#endif /* SERVER_HPP */
