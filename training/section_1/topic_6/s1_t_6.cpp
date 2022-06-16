#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "server.hpp"

using namespace std;

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

vector<string> SplitIntoWords(const string &text) {
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

void SearchServer::SetStopWords(const string &text) {
    for (const string &word : SplitIntoWords(text)) {
        stop_words_.insert(word);
    }
}

void SearchServer::AddDocument(int document_id, const string &document,
                               const vector<int> &ratings) {
    ++document_count_;
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string &word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    document_ratings_[document_id] = ratings;
}

vector<Document> SearchServer::FindTopDocuments(const string &raw_query) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query);

    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document &lhs, const Document &rhs) {
             return lhs.relevance > rhs.relevance || (lhs.relevance == rhs.relevance && lhs.rating > rhs.rating);
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

bool SearchServer::IsStopWord(const string &word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string &text) const {
    vector<string> words;
    for (const string &word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
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

Query SearchServer::ParseQuery(const string &text) const {
    Query query;
    for (const string &word : SplitIntoWords(text)) {
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

vector<int> SearchServer::ReadRatingsLine() {
    vector<int> result = {};
    uint32_t size;
    cin >> size;
    result.reserve(size);
    for (int i = 0; i < size; i++) {
        int v;
        cin >> v;
        result.push_back(v);
    } 
    ReadLine();
    return result;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string &word) const {
    return log(document_count_ * 1.0 / word_to_document_freqs_.at(word).size());
}

vector<Document> SearchServer::FindAllDocuments(const Query &query) const {
    map<int, double> document_to_relevance;
    for (const string &word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq =
            ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] :
             word_to_document_freqs_.at(word)) {
            document_to_relevance[document_id] +=
                term_freq * inverse_document_freq;
        }
    }

    for (const string &word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        const auto ratings = document_ratings_.at(document_id);
        const auto avg_rating = ComputeAverageRating(ratings);
        matched_documents.push_back({document_id, relevance, avg_rating});
    }
    return matched_documents;
}

int SearchServer::ComputeAverageRating(const vector<int> &ratings) {
    const auto size = static_cast<int>(ratings.size());
    if (!size) return 0;
    const auto result =
        accumulate(ratings.begin(), ratings.end(), 0.0,
                   [size](double avg, double v) { return avg + v / size; });
    return result;
}

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        const auto document = ReadLine();
        const vector<int> ratings = SearchServer::ReadRatingsLine();
        search_server.AddDocument(document_id, document, ratings);
    }

    return search_server;
}

int exec_main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();

    for (auto [document_id, relevance, avg_rating] :
         search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "s
             << "relevance = "s << relevance << ", "s
             << "rating = "s << avg_rating << " }"s << endl;
    }
    return 0;
}