
#include "./server.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <vector>

static const double THRESHOLD = 1e-6;

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

[[nodiscard]] bool SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    if (!IsValidDocumentId(document_id)) return false;
    if (documents_.count(document_id)) return false;  // Check if document with this id already exists

    const vector<string> words = SplitIntoWordsNoStop(document);
    if (!IsValidContent(words)) return false;

    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});

    return true;
}

// template <typename T>
optional<vector<Document>> SearchServer::FindTopDocuments(const string& raw_query, function<bool(int, DocumentStatus, int)> predicate) const {
    const Query query = ParseQuery(raw_query);
    if (!IsValidContent(query.plus_words)) return nullopt;
    if (!IsValidMinusWords(query.minus_words)) return nullopt;

    auto matched_documents = FindAllDocuments(query, predicate);

    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        if (abs(lhs.relevance - rhs.relevance) < THRESHOLD) {
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        }
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

optional<vector<Document>> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, [](int id, DocumentStatus status, [[maybe_unused]] int rating) -> bool {
        return status == DocumentStatus::ACTUAL;
    });
}

optional<vector<Document>> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int id, DocumentStatus doc_status, [[maybe_unused]] int rating) -> bool {
        return (doc_status == status);
    });
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(int index) const {
    int result = INVALID_DOCUMENT_ID;
    if (documents_.size() <= index) return result;

    auto i = 0;
    for (const auto& [id, _] : documents_) {
        if (index == i++) {
            result = id;
            break;
        }
    }
    return result;
}

set<std::string> SearchServer::GetStopWords() const {
    return stop_words_;
}

optional<tuple<vector<string>, DocumentStatus>> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
    if (!IsValidDocumentId(document_id)) return nullopt;

    const Query query = ParseQuery(raw_query);
    if (!IsValidContent(query.plus_words)) return nullopt;
    if (!IsValidMinusWords(query.minus_words)) return nullopt;

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

    tuple<vector<string>, DocumentStatus> result = {matched_words, documents_.at(document_id).status};
    return result;
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
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

// template <typename T>
vector<Document> SearchServer::FindAllDocuments(const Query& query, function<bool(int, DocumentStatus, int)> predicate) const {
    map<int, double> document_to_relevance;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            if (!documents_.count(document_id)) continue;
            const auto& document = documents_.at(document_id);
            if (predicate(document_id, document.status, document.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
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
        if (!IsValidMinusWords(word)) return false;
    }
    return true;
}

template <typename List>
bool SearchServer::IsValidContent(const List& content) {
    bool result = none_of(content.begin(), content.end(), [](const string& w) {
        return !IsValidWord(w);
    });
    return result;
}

bool SearchServer::IsValidDocumentId(int document_id) const {
    if (document_id < 0) return false;  // Check if ID is negative
    return true;
}
