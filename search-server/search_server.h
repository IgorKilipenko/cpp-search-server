#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <execution>
#include <functional>
#include <future>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "concurrent_map.h"
#include "document.h"
#include "paginator.h"
#include "string_processing.h"

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
constexpr const double THRESHOLD = 1e-6;

class SearchServer {
   public:
    using IdsConstIterator = vector<int>::const_iterator;
    using IdsIterator = vector<int>::iterator;

    SearchServer() = default;

    template <class Container>
    explicit SearchServer(const Container& stop_words);

    explicit SearchServer(const string_view stop_words_text);

    explicit SearchServer(const string& stop_words_text);

    /// Add new document to the search server's internal database
    void AddDocument(int document_id, const string_view document, DocumentStatus status = DocumentStatus::ACTUAL, const vector<int>& ratings = {});

    /// Find most matched documents for request
    template <typename ExecutionPolicy, typename T = function<bool(int, DocumentStatus, int)>,
              std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                                   std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                               bool> = true>
    vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, T predicate) const;

    /// Find most matched documents for request
    template <typename T = function<bool(int, DocumentStatus, int)>>
    vector<Document> FindTopDocuments(const string_view raw_query, T predicate) const;

    template <typename ExecutionPolicy, std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                                                             std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                                                         bool> = true>
    vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query) const;

    vector<Document> FindTopDocuments(const string_view raw_query) const;

    template <typename ExecutionPolicy, std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                                                             std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                                                         bool> = true>
    vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, DocumentStatus status) const;

    vector<Document> FindTopDocuments(const string_view raw_query, DocumentStatus status) const;

    /// Get total number of documents in internal database
    int GetDocumentCount() const;

    tuple<vector<string_view>, DocumentStatus> MatchDocument(const string_view raw_query, int document_id) const;

    template <typename ExecutionPolicy>
    tuple<vector<string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const string_view raw_query, int document_id) const;

    set<std::string, std::less<>> GetStopWords() const;

    IdsConstIterator begin() const;

    IdsConstIterator end() const;

    map<string_view, double> GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

    template <typename ExecutionPolicy, std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                                                             std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                                                         bool> = true>
    void RemoveDocument(ExecutionPolicy policy, int document_id);

    void RemoveDuplicates();

   private:
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::ACTUAL;
    };
    struct QueryWord {
        string_view data;
        bool is_minus = false;
        bool is_stop = false;
    };

    struct Query {
        vector<string_view> plus_words;
        vector<string_view> minus_words;

        template <typename ExecutionPolicy>
        static void MakeUnique(ExecutionPolicy&& policy, vector<string_view>& words) {
            std::sort(policy, words.begin(), words.end());
            auto last = std::unique(policy, words.begin(), words.end());
            words.erase(last, words.end());
        }

        static void MakeUnique(vector<string_view>& words) {
            Query::MakeUnique(std::execution::seq, words);
        }

        template <typename ExecutionPolicy>
        void MakeUnique(ExecutionPolicy&& policy) {
            Query::MakeUnique(policy, plus_words);
            Query::MakeUnique(policy, minus_words);
        }

        void MakeUnique() {
            MakeUnique(std::execution::seq);
        }
    };

    set<string, std::less<>> stop_words_;
    map<string, map<int, double>, std::less<>> word_to_document_freqs_;
    map<int, map<string_view, double>> document_to_words_freqs_;
    map<int, DocumentData> documents_;
    vector<int> document_ids_;
    map<size_t, set<int>> hash_content_;

    bool IsStopWord(const string_view word) const;

    vector<string_view> SplitIntoWordsNoStop(const string_view text) const;

    static int ComputeAverageRating(const vector<int>& ratings);

    QueryWord ParseQueryWord(const string_view text) const;

    Query ParseQuery(const string_view text, bool make_unique = true) const;

    double ComputeWordInverseDocumentFreq(const string_view word) const;

    template <typename ExecutionPolicy, typename T = function<bool(int, DocumentStatus, int)>,
              std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                                   std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                               bool> = true>
    vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const Query& query, T predicate) const;

    template <typename T = function<bool(int, DocumentStatus, int)>>
    vector<Document> FindAllDocuments(const Query& query, T predicate) const;

    static bool IsValidWord(const string_view word);

    template <typename ExecutionPolicy>
    static void EraseFromWordToDocumentFreqs(ExecutionPolicy&& policy, int id, vector<string_view>&& words,
                                             map<std::string, map<int, double>, std::less<>>& word_to_document_freqs);
};

// ----------------------------------------------------------------
// Helper methods
// ----------------------------------------------------------------

/// Exceptions safety version of AddDocument
void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status = DocumentStatus::ACTUAL,
                 const vector<int>& ratings = {});

/// Exceptions safety version of FindTopDocuments
void FindTopDocuments(const SearchServer& search_server, const string& raw_query);

/// Exceptions safety version of FindTopDocuments
void MatchDocuments(const SearchServer& search_server, const string& query);

/// Pagenate mathing documents
template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

/// Remove all documents duplicates from database
void RemoveDuplicates(SearchServer& search_server);

template <typename TDict, typename TKey>
auto EraseFromContainer(TKey id, TDict& container) {
    auto ptr = container.find(id);
    if (ptr != container.end()) {
        return container.erase(ptr);
    }
    return container.end();
}

template <typename ExecutionPolicy, template <typename, typename> class Container, typename Value, typename Allocator = std::allocator<Value>>
auto EraseFromContainer(ExecutionPolicy&& policy, Value id, Container<Value, Allocator>& container) {
    auto ptr = find(policy, container.begin(), container.end(), id);
    if (ptr != container.end()) {
        return container.erase(ptr);
    }
    return container.end();
}

template <typename T>
auto EraseFromContainer(T id, vector<T>& container) {
    return EraseFromContainer(std::execution::seq, id, container);
}

template <typename ExecutionPolicy, template <typename, typename> class Container, typename Key, typename Value>
auto EraseFromDictionary(ExecutionPolicy&& policy, Key id, Container<Key, Value>& container) {
    auto ptr = std::find_if(policy, container.begin(), container.end(), [id](auto item) {
        return item.first == id;
    });
    if (ptr != container.end()) {
        return container.erase(ptr);
    }
    return container.end();
}

template <template <typename, typename> class Container, typename Key, typename Value>
auto EraseFromDictionary(Key id, Container<Key, Value>& container) {
    EraseFromDictionary(std::execution::seq, id, container);
}

// ----------------------------------------------------------------
// SearchServer template members implementation
// ----------------------------------------------------------------

template <class Container>
SearchServer::SearchServer(const Container& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words.begin(), stop_words.end())) {
    if (any_of(stop_words_.begin(), stop_words_.end(), [](const auto word) {
            return !IsValidWord(word);
        })) {
        throw invalid_argument("Invalid stop words"s);
    }
}

template <typename ExecutionPolicy, typename T,
          std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                               std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                           bool>>
vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, T predicate) const {
    auto query = ParseQuery(raw_query, true);
    if (query.plus_words.empty()) {
        return {};
    }

    query.MakeUnique();
    auto matched_documents = FindAllDocuments(policy, std::move(query), predicate);

    sort(policy, matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        return lhs.relevance > rhs.relevance || (std::abs(lhs.relevance - rhs.relevance) < THRESHOLD && lhs.rating > rhs.rating);
    });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename ExecutionPolicy, typename T,
          std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                               std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                           bool>>
vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy, const Query& query, T predicate) const {
    if (query.plus_words.empty()) {
        return {};
    }

    ConcurrentSet<int> cm_exclude_doc_ids{word_to_document_freqs_.size() * (query.minus_words.empty() ? 0ul : 1ul)};
    std::for_each(policy, query.minus_words.begin(), query.minus_words.end(), [this, &cm_exclude_doc_ids](const string_view minus_word) {
        auto ptr = word_to_document_freqs_.find(minus_word);
        if (ptr == word_to_document_freqs_.end()) {
            return;
        }

        const auto& words = ptr->second;

        for (const auto& [document_id, _] : words) {
            cm_exclude_doc_ids[document_id].ref_to_value = document_id;
        }
    });

    std::set<int> exclude_doc_ids = cm_exclude_doc_ids.BuildOrdinarySet();
    ConcurrentMap<int, double> document_to_relevance(word_to_document_freqs_.size());
    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
                  [this, &document_to_relevance, predicate, exclude_doc_ids](const string_view word) {
                      auto ptr = word_to_document_freqs_.find(word);
                      if (ptr == word_to_document_freqs_.end()) {
                          return;
                      }

                      const auto& words = ptr->second;

                      const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                      for (const auto& [document_id, term_freq] : words) {
                          if (!exclude_doc_ids.empty() && exclude_doc_ids.count(document_id)) {
                              continue;
                          }
                          const auto& document_data = documents_.at(document_id);
                          if (predicate(document_id, document_data.status, document_data.rating)) {
                              document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
                          }
                      }
                  });

    const auto docs = document_to_relevance.BuildOrdinaryVector();
    std::vector<Document> matched_documents(docs.size());
    std::transform(policy, std::make_move_iterator(docs.begin()), std::make_move_iterator(docs.end()), matched_documents.begin(),
                   [this](const auto item) -> Document {
                       return {item.first, item.second, documents_.at(item.first).rating};
                   });

    return matched_documents;
}

template <typename T>
vector<Document> SearchServer::FindAllDocuments(const Query& query, T predicate) const {
    return FindAllDocuments(std::execution::seq, query, predicate);
}

template <typename ExecutionPolicy>
void SearchServer::EraseFromWordToDocumentFreqs(ExecutionPolicy&& policy, int id, vector<string_view>&& words,
                                                map<std::string, map<int, double>, std::less<>>& word_to_document_freqs) {
    std::for_each(policy, words.begin(), words.end(), [&](const string_view cur_word) {
        auto docs_ptr = word_to_document_freqs.find(cur_word);
        if (docs_ptr == word_to_document_freqs.end() || docs_ptr->second.empty()) {
            return;
        }
        auto& ids_freq = docs_ptr->second;
        auto ptr = find_if(policy, ids_freq.begin(), ids_freq.end(), [id](const pair<int, double>& item) {
            return item.first == id;
        });
        if (ptr == ids_freq.end()) {
            return;
        }
        ids_freq.erase(ptr);
    });
}

template <typename ExecutionPolicy>
tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, const string_view raw_query, int document_id) const {
    auto word_freqs_ptr = document_to_words_freqs_.find(document_id);
    if (word_freqs_ptr == document_to_words_freqs_.end()) {
        throw out_of_range("No document with id: "s + to_string(document_id));
    }

    const auto& words = word_freqs_ptr->second;

    Query query = ParseQuery(raw_query, false);
    tuple<vector<string_view>, DocumentStatus> result{{}, documents_.at(document_id).status};

    query.MakeUnique(query.minus_words);
    if (std::any_of(query.minus_words.begin(), query.minus_words.end(), [&words](const auto minus_word) {
            return words.count(minus_word);
        })) {
        return result;
    }

    query.MakeUnique(query.plus_words);

    auto& matched_words = std::get<0>(result);
    matched_words.reserve(query.plus_words.size());
    std::mutex mutex;
    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(), [&mutex, &words, &matched_words](const auto plus_word) {
        if (words.count(plus_word)) {
            std::lock_guard<std::mutex> lock_guard(mutex);
            matched_words.push_back(plus_word);
        }
    });

    return result;
}

template <typename T>
vector<Document> SearchServer::FindTopDocuments(const string_view raw_query, T predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template <typename ExecutionPolicy, std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                                                         std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                                                     bool>>
vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query, [status](int id, DocumentStatus doc_status, [[maybe_unused]] int rating) -> bool {
        return (doc_status == status);
    });
}

template <typename ExecutionPolicy, std::enable_if_t<std::is_convertible<ExecutionPolicy, std::execution::sequenced_policy>::value ||
                                                         std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value,
                                                     bool>>
vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}