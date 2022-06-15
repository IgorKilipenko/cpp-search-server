#ifndef SERVER_HPP
#define SERVER_HPP

#ifndef EXPORT
#define EXPORT
#endif

#include <string>
#include <utility>
#include <vector>
#include <set>

using namespace std;

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
    struct DocumentContent {
        int id = 0;
        vector<string> words;
    };
    vector<DocumentContent> documents_;
    set<string> stop_words_;

    bool IsStopWord(const string& word) const;
    vector<string> SplitIntoWordsNoStop(const string& text) const;
    Query ParseQuery(const string& text) const;
    vector<Document> FindAllDocuments(const Query& query) const;
    static int MatchDocument(const DocumentContent& content, const Query& query_words);
};

EXPORT int exec_main();

#endif /* SERVER_HPP */
