#include <algorithm>
#include <cassert>
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
                               DocumentStatus status,
                               const vector<int> &ratings) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string &word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id,
                           DocumentInfo{ComputeAverageRating(ratings), status});
}

vector<Document> SearchServer::FindTopDocuments(const string &raw_query,
                                                DocumentStatus status) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, status);

    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document &lhs, const Document &rhs) {
             return lhs.relevance > rhs.relevance;
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
    uint32_t size;
    cin >> size;
    vector<int> result(size, 0);
    result.reserve(size);
    for (auto &r : result) {
        cin >> r;
    }
    ReadLine();
    return result;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string &word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

vector<Document> SearchServer::FindAllDocuments(const Query &query,
                                                DocumentStatus status) const {
    map<int, double> document_to_relevance;
    for (const string &word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq =
            ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] :
             word_to_document_freqs_.at(word)) {
            if (documents_.count(document_id) &&
                static_cast<int>(documents_.at(document_id).status) ^
                    static_cast<int>(status))
                continue;
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
        const auto avg_rating = documents_.at(document_id).rating;
        matched_documents.push_back({document_id, relevance, avg_rating});
    }
    return matched_documents;
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(
    const string &raw_query, int document_id) const {
    assert(documents_.count(document_id));
    //if (!document_info_.count(document_id)) return {{}};

    const Query query = ParseQuery(raw_query);
    const auto [_, status] = documents_.at(document_id);

    set<string> plus_words(query.plus_words.begin(), query.plus_words.end());
    set<string> matched_words{};

    for (const string &minus_word : query.minus_words) {
        if (plus_words.count(minus_word)) plus_words.erase(minus_word);
        if (word_to_document_freqs_.count(minus_word) == 0) continue;
        const auto &ids = word_to_document_freqs_.at(minus_word);
        if (ids.count(document_id)) return {{}, status};
    }

    for (const string &plus_word : plus_words) {
        if (word_to_document_freqs_.count(plus_word) == 0) continue;
        const auto &ids = word_to_document_freqs_.at(plus_word);
        if (ids.count(document_id)) matched_words.insert(plus_word);
    }

    return {vector<string>(matched_words.begin(), matched_words.end()), status};
}

int SearchServer::ComputeAverageRating(const vector<int> &ratings) {
    const auto size = static_cast<int>(ratings.size());
    if (!size) return 0;
    const auto result =
        accumulate(ratings.begin(), ratings.end(), 0.0,
                   [size](double avg, double v) { return avg + v / size; });
    return result;
}

int SearchServer::GetDocumentCount(void) const { return documents_.size(); }

/*
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
*/

void PrintDocument(const Document &document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

void PrintMatchDocumentResult(int document_id, const vector<string> &words,
                              DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const string &word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

void mock_data() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s,
                              DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,
                              DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s,
                              DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,
                              DocumentStatus::BANNED, {9});

    cout << "ACTUAL:"s << endl;
    for (const Document &document :
         search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    cout << "BANNED:"s << endl;
    for (const Document &document : search_server.FindTopDocuments(
             "пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }
}

void mock_data_for_MatchDocument() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s,
                              DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,
                              DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s,
                              DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,
                              DocumentStatus::BANNED, {9});

    const int document_count = search_server.GetDocumentCount();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        const auto [words, status] =
            search_server.MatchDocument("пушистый кот"s, document_id);
        PrintMatchDocumentResult(document_id, words, status);
    }
}

int exec_main() {
    mock_data_for_MatchDocument();
    return 0;
}