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
    int relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text);
    void AddDocument(int document_id, const string& document);
    vector<Document> FindTopDocuments(const string& raw_query) const;

private:
    DocumentsIndexTable documents_table_;
    set<string> stop_words_;

    bool IsStopWord(const string& word) const;
    vector<string> SplitIntoWordsNoStop(const string& text) const;
    Query ParseQuery(const string& text) const;
    vector<Document> FindAllDocuments(const Query& query) const;
    static map<int,int> MatchDocument(const DocumentsIndexTable& doc_ids_table, const Query& query);
};

EXPORT int exec_main();

#endif /* SERVER_HPP */
