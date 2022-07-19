#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <future>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "string_processing.h"

using namespace std;

// ----------------------------------------------------------------
// SearchServer implementation
// ----------------------------------------------------------------

SearchServer::SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
{}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    auto& words_container = document_to_words_freqs_[document_id];
    string hash_sep = ""s;
    for (const string& word : words) {
        double& curr_freq = word_to_document_freqs_[word][document_id];
        curr_freq += inv_word_count;
        words_container.insert({word, curr_freq});
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);

    auto hash = BuildHash<double>(words_container, {}, ","s);
    hash_content_[hash].insert(document_id);
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

SearchServer::IdsConstIterator SearchServer::begin() const {
    return document_ids_.begin();
}

SearchServer::IdsConstIterator SearchServer::end() const {
    return document_ids_.end();
}

set<std::string> SearchServer::SearchServer::GetStopWords() const {
    return stop_words_;
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument([[maybe_unused]] std::execution::sequenced_policy policy, const string& raw_query,
                                                                  int document_id) const {
    const auto query = ParseQuery(raw_query);

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

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(std::execution::parallel_policy policy, const string_view raw_query,
                                                                  int document_id) const {
    if (word_to_document_freqs_.empty()) {
        return {};
    }

    auto word_freqs_ptr = document_to_words_freqs_.find(document_id);
    if (word_freqs_ptr == document_to_words_freqs_.end()) {
        throw out_of_range("No document with id: "s + to_string(document_id));
    }

    const Query query = ParseQuery(raw_query);
    if (query.plus_words.empty()) {
        return {};
    }

    if (std::any_of(query.minus_words.begin(), query.minus_words.end(), [&](auto minus_word) {
            return word_freqs_ptr->second.count(move(minus_word));
        })) {
        return {};
    }

    vector<string> matched_words;
    const auto& word_freqs = word_freqs_ptr->second;
    matched_words.reserve(query.plus_words.size());
    std::mutex mutex;
    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(), [&word_freqs, &mutex, &matched_words](auto plus_word) {
        if (word_freqs.count(plus_word)) {
            std::lock_guard<std::mutex> guard(mutex);
            matched_words.push_back(std::move(plus_word));
        }
    });
    return {std::move(matched_words), documents_.at(document_id).status};
}

const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const map<string, double> invalid_result{};
    if (document_to_words_freqs_.empty() || !document_to_words_freqs_.count(document_id)) {
        return invalid_result;
    }

    const auto& word_freqs = document_to_words_freqs_.at(document_id);
    return word_freqs;
}

template <>
void SearchServer::RemoveDocument(std::execution::sequenced_policy policy, int document_id) {
    if (document_ids_.empty() || !documents_.count(document_id)) {
        return;
    }

    EraseFromContainer(document_id, document_ids_);
    EraseFromContainer(document_id, documents_);
    auto doc_words_ptr = document_to_words_freqs_.find(document_id);
    if (doc_words_ptr == document_to_words_freqs_.end() || word_to_document_freqs_.empty()) {
        return;  // Empty document or empty word_to_document_freqs container
    }

    const map<string, double>& words = doc_words_ptr->second;
    for (auto& [cur_word, _] : words) {
        auto docs_ptr = word_to_document_freqs_.find(cur_word);
        if (docs_ptr == word_to_document_freqs_.end() || docs_ptr->second.empty()) {
            continue;
        }
        auto& ids_freq = docs_ptr->second;
        auto ptr = find_if(ids_freq.begin(), ids_freq.end(), [document_id](const pair<int, double>& item) {
            return item.first == document_id;
        });
        if (ptr == ids_freq.end()) {
            continue;
        }
        ids_freq.erase(ptr);
    }

    document_to_words_freqs_.erase(doc_words_ptr);

    for (auto& [_, ids] : hash_content_) {
        if (ids.empty()) {
            continue;
        }
        EraseFromContainer(document_id, ids);
    }
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq, document_id);
}

template <>
void SearchServer::RemoveDocument(std::execution::parallel_policy policy, int document_id) {
    if (document_ids_.empty() || !documents_.count(document_id)) {
        return;
    }
    auto doc_words_ptr = document_to_words_freqs_.find(document_id);
    if (doc_words_ptr == document_to_words_freqs_.end() || word_to_document_freqs_.empty()) {
        return;  // Empty document or empty word_to_document_freqs container
    }

    set<string> exclude_words{};
    auto hash = BuildHash(doc_words_ptr->second, exclude_words);
    auto& hash_ids = hash_content_.at(hash);
    EraseFromContainer(document_id, hash_ids);

    vector<string> words{doc_words_ptr->second.size()};
    std::transform(policy, doc_words_ptr->second.begin(), doc_words_ptr->second.end(), words.begin(), [](const auto& item) {
        return item.first;
    });

    EraseFromWordToDocumentFreqs(policy, document_id, std::move(words), word_to_document_freqs_);

    EraseFromContainer(document_id, document_ids_);
    EraseFromContainer(document_id, documents_);
    document_to_words_freqs_.erase(doc_words_ptr);
}

void SearchServer::RemoveDuplicates() {
    for (auto& [_, ids] : hash_content_) {
        if (ids.empty() || ids.size() == 1) {
            continue;
        }
        for (auto ptr = next(ids.begin()), last = ids.end(); ptr != last;) {
            int id = *ptr;
            ptr = ids.erase(ptr);
            RemoveDocument(id);
            cout << "Found duplicate document "s << id << endl;
        }
    }
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string_view text) const {
    vector<string> words;
    for (const string_view word_v : SplitIntoWords(text)) {
        string word{word_v};
        if (!IsValidWord(word)) {
            throw invalid_argument("Word "s + word + " is invalid"s);
        }
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

SearchServer::QueryWord SearchServer::ParseQueryWord(const string_view text) const {
    if (text.empty()) {
        throw invalid_argument("Query word is empty"s);
    }
    string word(text);
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        word = text.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw invalid_argument("Query word "s + static_cast<string>(text) + " is invalid");
    }

    return {move(word), is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const string_view text) const {
    Query result;
    for (const string_view word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(string(word));
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            } else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

bool SearchServer::IsValidWord(const string& word) {
    return none_of(std::execution::seq, word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

// ----------------------------------------------------------------
// Helper methods implementation
// ----------------------------------------------------------------

void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const exception& e) {
        cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query) {
    cout << "Результаты поиска по запросу: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const exception& e) {
        cout << "Ошибка поиска: "s << e.what() << endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const string& query) {
    try {
        cout << "Матчинг документов по запросу: "s << query << endl;
        const int document_count = search_server.GetDocumentCount();
        for (int index = 0; index < document_count; ++index) {
            const int document_id = *(search_server.begin() + index);
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const exception& e) {
        cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
    }
}

void RemoveDuplicates(SearchServer& search_server) {
    search_server.RemoveDuplicates();
}
