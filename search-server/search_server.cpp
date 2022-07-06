#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
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
    string hash_buffer_str;
    string hash_sep = ""s;
    for (const string& word : words) {
        if (stop_words_.empty() || !stop_words_.count(word)) {
            hash_buffer_str += hash_sep + word;
            if (hash_sep.empty()) {
                hash_sep = ",";
            }
        }
        double& curr_freq = word_to_document_freqs_[word][document_id];
        curr_freq += inv_word_count;
        words_container.insert({word, curr_freq});
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);

    auto hash = BuildHash<double>(words_container, stop_words_, ","s);
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

const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const map<string, double> invalid_result{};
    if (document_to_words_freqs_.empty() || !document_to_words_freqs_.count(document_id)) {
        return invalid_result;
    }

    const auto& word_freqs = document_to_words_freqs_.at(document_id);
    return word_freqs;
}

void SearchServer::RemoveDocument(int document_id) {
    //// map<string, map<int, double>> word_to_document_freqs_;
    //// map<int, map<string, double>> document_to_words_freqs_;
    //// map<int, DocumentData> documents_;
    //// vector<int> document_ids_;

    if (document_ids_.empty() || !documents_.count(document_id)) {
        return;
    }

    EraseFromContainer(document_id, document_ids_);
    EraseFromContainer(document_id, documents_);
    auto doc_words_ptr = EraseFromContainer(document_id, document_to_words_freqs_);
    if (doc_words_ptr == document_to_words_freqs_.end() || word_to_document_freqs_.empty()) {
        return;  // Empty document or empty word_to_document_freqs container
    }

    // vector<map<const int, double>::const_iterator> ids_freq_for_erase;
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
        // ids_freq_for_erase.push_back(ptr);
        ids_freq.erase(ptr);
    }

    // vector<vector<int>::const_iterator> ids_freq_for_erase;
    for (auto& [_, ids] : hash_content_) {
        if (ids.empty()) {
            continue;
        }
        /*auto ptr = ids.begin();
        while (ptr != ids.end()) {
            ptr = find(ptr, ids.end(), document_id);
            if (ptr != ids.end()) {
                ids.erase(ptr);
            }
        }*/
        EraseFromContainer(document_id, ids);
    }
}

/*
Дубликатами считаются документы, у которых наборы встречающихся слов совпадают. Совпадение частот необязательно.
Порядок слов неважен, а стоп-слова игнорируются.
Функция должна использовать только доступные к этому моменту методы поискового сервера.
При обнаружении дублирующихся документов функция
должна удалить документ с большим id из поискового сервера, и при этом сообщить id удалённого документа в
соответствии с форматом выходных данных, приведённым ниже.
*/
void SearchServer::RemoveDuplicates() {
    /*
    vector<map<int, map<string, double>>::iterator> buffer;
    for (const auto& word_freqs : document_to_words_freqs_) {
        auto ptr = document_to_words_freqs_.begin();
        while (ptr != document_to_words_freqs_.end()) {
            ptr = find_if(ptr, document_to_words_freqs_.end(), [&word_freqs](const auto& item) {
                auto ws = item.second;
                return equal(ws.begin(), ws.end(), word_freqs.second.begin(), word_freqs.second.end());
            });
            if (ptr != document_to_words_freqs_.end()) {
                buffer.push_back(ptr);
                ++ptr;
            }
        }
    }
    for (auto ptr : buffer) {
        document_to_words_freqs_.erase(ptr);
    }*/

    for (const auto& [_, ids] : hash_content_) {
        if (ids.empty() || ids.size() == 1) {
            continue;
        }
        /*for (int i = 1; i < ids.size(); ++i) {
            RemoveDocument(ids[i]);
        }*/
        /*auto first = next(ids.begin());
        auto last = ids.end();
        for (auto prt = first; prt != last; prt++) {
            cout << "Found duplicate document "s  << *prt << endl;
            RemoveDocument(*prt);
        }*/
        vector<int> ids_for_erase;
        ids_for_erase.reserve(ids.size()-1);
        auto first = next(ids.begin());
        auto last = ids.end();
        for (auto ptr = first; ptr != last; ptr++) {
            //cout << "Found duplicate document "s  << *prt << endl;
            //RemoveDocument(*prt);
            ids_for_erase.push_back(*ptr);
        }

        for (int id : ids_for_erase) {
            RemoveDocument(id);
            cout << "Found duplicate document "s  << id << endl;
        }
    }
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
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

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
    if (text.empty()) {
        throw invalid_argument("Query word is empty"s);
    }
    string word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw invalid_argument("Query word "s + text + " is invalid");
    }

    return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query result;
    for (const string& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
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
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

int SearchServer::GetDocumentId(int index) const {
    return *(begin() + index);
}

SearchServer::IdsIterator SearchServer::begin() {
    return document_ids_.begin();
}

SearchServer::IdsIterator SearchServer::end() {
    return document_ids_.end();
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
