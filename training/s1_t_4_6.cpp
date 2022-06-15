#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <cassert>

#include "server.hpp"

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;



static string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

static int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

static vector<string> SplitIntoWords(const string& text) {
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

void SearchServer::AddDocument(int document_id, const string& document) {
    bool is_new_document = true;
    for (const auto& [_, ids] : documents_table_) {
        if (!is_new_document) break;
        is_new_document = !ids.count(document_id);
    }
    if (is_new_document) documents_count_ += 1;

    const vector<string> words = SplitIntoWordsNoStop(document);
    for (const auto& w : words) {
        auto& ids = documents_table_[w];
        ids.insert(document_id);
        const auto freq = culcWordTermFrequency(w, words);
        word_to_document_freqs_[w][document_id] = freq;
        //inverse_document_freqs_[w] = culcWordInverseDocumentFrequency(w);
    }
}

double SearchServer::culcWordTermFrequency(const string &word, const vector<string>& words) const {
    const auto result = static_cast<double>(count(words.begin(), words.end(), word)) / words.size();
    return result;
}

double SearchServer::culcWordInverseDocumentFrequency(const string &word) const {
    const auto ids_count = documents_table_.count(word) ? documents_table_.at(word).size() : 0;
    const auto freq = static_cast<double>(documents_count_) / ids_count;
    const auto result = log(static_cast<double>(documents_count_) / ids_count);
    return result;
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query);

    sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
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

Query SearchServer::ParseQuery(const string& text) const {
    set<string> query_words;
    set<string> exclude_words;
    for (const string& word : SplitIntoWordsNoStop(text)) {
        if (word.empty()) continue;
        if (word[0] == '-') {
            exclude_words.insert(word.substr(1));
        }else {
            query_words.insert(word);
        }
    }
    Query result = {query_words, exclude_words};
    return result;
}

vector<Document> SearchServer::FindAllDocuments(const Query& query) const {
    const auto matched_documents = MatchDocument(documents_table_, query);
    vector<Document> result{};
    for (const auto& val : matched_documents) {
        Document doc = {val.first, val.second};
        result.push_back(doc);
    }
    return result;
}


map<int,double> SearchServer::MatchDocument(const DocumentsIndexTable& doc_ids_table, const Query& query) const {
    map<int,double> result{};
    if (query.words.empty()) {
        return result;
    }

    // Find all documents matching the specified query words with documents contains exclude_words
    map<string,set<int>> matched;
    for (const auto& word : query.words) {
        if (doc_ids_table.count(word) == 0) continue;
        const auto& ids = doc_ids_table.at(word);
        matched[word].insert(ids.begin(), ids.end());
    }

    set<int> erase_ids{};
    for (const auto& [word, ids] : matched) {
        for (const auto id : ids) {
            if (!query.exclude_words.count(word)) {
                assert(word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(id));
                const auto freq =  word_to_document_freqs_.at(word).at(id);
                result[id] = freq * culcWordInverseDocumentFrequency(word);
            } else {
                erase_ids.insert(id);
            }
        }
    }
    for (const auto eid: erase_ids) {
        result.erase(eid);
    }
    return result;
}


static SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

extern int exec_main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
    return 0;
}
