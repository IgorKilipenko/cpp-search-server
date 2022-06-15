#ifndef SERVER_HPP
#define SERVER_HPP

#ifndef EXPORT
#define EXPORT
#endif

#include <string>
#include <utility>
#include <vector>
#include <set>
#include <map>

using namespace std;

using DocumentIndex = pair<string, set<int>>;
using DocumentsIndexTable = map<string, set<int>>;

struct Query {
    set<string> words;
    set<string> exclude_words;
};

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text);
    void AddDocument(int document_id, const string& document);
    vector<Document> FindTopDocuments(const string& raw_query) const;

private:
    DocumentsIndexTable documents_table_;
    size_t documents_count_ = 0;
    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    //map<string, double> inverse_document_freqs_;

    bool IsStopWord(const string& word) const;
    vector<string> SplitIntoWordsNoStop(const string& text) const;
    Query ParseQuery(const string& text) const;
    vector<Document> FindAllDocuments(const Query& query) const;
    map<int,double> MatchDocument(const DocumentsIndexTable& doc_ids_table, const Query& query) const;
    double culcWordTermFrequency(const string &word, const vector<string>& words) const;
    double culcWordInverseDocumentFrequency(const string &word) const;
};

EXPORT int exec_main();

#endif /* SERVER_HPP */
