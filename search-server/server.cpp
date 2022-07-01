#include "server.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

extern vector<string> SplitIntoWords(const string& text) {
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

SearchServer::SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
{}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    if (!IsValidDocumentId(document_id)) {
        throw invalid_argument("Invalid document ID."s);
    }
    if (documents_.count(document_id)) {
        throw invalid_argument("Document with this ID already exists."s);
    }

    const vector<string> words = SplitIntoWordsNoStop(document);
    if (!IsValidContent(words)) {
        throw invalid_argument("Invalid document content."s);
    }

    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    documents_ids_queue_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int id, DocumentStatus doc_status, [[maybe_unused]] int rating) -> bool {
        return (doc_status == status);
    });
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(int index) const {
    if (index < 0 || documents_ids_queue_.size() <= index) {
        throw out_of_range("Invalid document index. " + "Index: "s + to_string(index) + " out of range."s);
    }
    return documents_ids_queue_[index];
}

set<std::string> SearchServer::GetStopWords() const {
    return stop_words_;
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
    if (!IsValidDocumentId(document_id)) {
        throw invalid_argument("Invalid document ID."s);
    }

    const Query query = ParseQuery(raw_query);

    vector<string> matched_words;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    return {matched_words, documents_.at(document_id).status};
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

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }

    const int rating_sum = accumulate(ratings.begin(), ratings.end(), 0, [](int summ, int rating) {
        return summ + rating;
    });

    return rating_sum / static_cast<int>(ratings.size());
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

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query query;
    for (const string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                if (!IsValidMinusWords(query_word.data)) {
                    throw invalid_argument("Invalid minus-word.");
                }
                query.minus_words.insert(query_word.data);
            } else {
                if (!IsValidWord(query_word.data)) {
                    throw invalid_argument("Invalid minus-word.");
                }
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

bool SearchServer::IsValidWord(const string& word) {
    const auto containMinusChars = [](const string& word) -> bool {
        std::regex self_regex("^[\\-]*.*\\s+\\-+.*", std::regex_constants::ECMAScript);
        return std::regex_search(word, self_regex);
    };
    const auto containSpecialChars = [](const string& word) -> bool {
        std::regex self_regex("[\\u0000-\\u001F]+", std::regex_constants::ECMAScript);
        return std::regex_search(word, self_regex);
    };

    return !containMinusChars(word) && !containSpecialChars(word);
}

bool SearchServer::IsValidMinusWords(const string& word) {
    std::regex self_regex("^[^\\-\\s]+", std::regex_constants::ECMAScript);
    return std::regex_search(word, self_regex) && IsValidWord(word);
}

bool SearchServer::IsValidMinusWords(const set<string>& words) {
    for (const auto& word : words) {
        if (!IsValidMinusWords(word)) {
            return false;
        }
    }
    return true;
}

bool SearchServer::IsValidDocumentId(int document_id) const {
    if (document_id < 0) {
        return false;  // Check if ID is negative
    }
    return true;
}

