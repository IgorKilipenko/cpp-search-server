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

const int MAX_RESULT_DOCUMENT_COUNT = 5;
constexpr const double THRESHOLD = 1e-6;

template <class ExecutionPolicy>
using IsExecutionPolicy = std::is_execution_policy<std::decay_t<ExecutionPolicy>>;

template <class ExecutionPolicy>
using EnableForExecutionPolicy = typename std::enable_if_t<IsExecutionPolicy<ExecutionPolicy>::value, bool>;

class SearchServer {
   public:
    using IdsConstIterator = std::vector<int>::const_iterator;
    using IdsIterator = std::vector<int>::iterator;

    SearchServer() = default;

    template <class Container>
    explicit SearchServer(const Container& stop_words);

    explicit SearchServer(const std::string_view stop_words_text);

    explicit SearchServer(const std::string& stop_words_text);

    /// Add new document to the search server's internal database
    void AddDocument(int document_id, const std::string_view document, DocumentStatus status = DocumentStatus::ACTUAL,
                     const std::vector<int>& ratings = {});

    /// Find most matched documents for request
    template <typename ExecutionPolicy, typename DocumentPredicate, EnableForExecutionPolicy<ExecutionPolicy> = true>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentPredicate predicate) const;

    /// Find most matched documents for request
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentPredicate predicate) const;

    template <class ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy> = true>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query) const;

    std::vector<Document> FindTopDocuments(const std::string_view raw_query) const;

    template <class ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy> = true>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const;

    /// Get total number of documents in internal database
    int GetDocumentCount() const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query, int document_id) const;

    template <typename ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy> = true>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string_view raw_query,
                                                                            int document_id) const;

    std::set<std::string, std::less<>> GetStopWords() const;

    IdsConstIterator begin() const;

    IdsConstIterator end() const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

    template <typename ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy> = true>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id);

    void RemoveDuplicates();

   private:
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::ACTUAL;
    };
    struct QueryWord {
        std::string_view data;
        bool is_minus = false;
        bool is_stop = false;
    };

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;

        template <typename ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy> = true>
        static void MakeUnique(ExecutionPolicy&& policy, std::vector<std::string_view>& words) {
            std::sort(policy, words.begin(), words.end());
            auto last = std::unique(policy, words.begin(), words.end());
            words.erase(last, words.end());
        }

        static void MakeUnique(std::vector<std::string_view>& words) {
            Query::MakeUnique(std::execution::seq, words);
        }

        template <typename ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy> = true>
        void MakeUnique(ExecutionPolicy&& policy) {
            Query::MakeUnique(policy, plus_words);
            Query::MakeUnique(policy, minus_words);
        }

        void MakeUnique() {
            MakeUnique(std::execution::seq);
        }
    };

    std::set<std::string, std::less<>> stop_words_;
    std::map<std::string, std::map<int, double>, std::less<>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_to_words_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;
    std::map<size_t, std::set<int>> hash_content_;

    bool IsStopWord(const std::string_view word) const;

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    QueryWord ParseQueryWord(const std::string_view text) const;

    Query ParseQuery(const std::string_view text, bool make_unique = true) const;

    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    template <typename ExecutionPolicy, typename DocumentPredicate, EnableForExecutionPolicy<ExecutionPolicy> = true>
    std::vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const Query& query, DocumentPredicate predicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicate) const;

    static bool IsValidWord(const std::string_view word);

    template <typename ExecutionPolicy>
    static void EraseFromWordToDocumentFreqs(ExecutionPolicy&& policy, int id, std::vector<std::string_view>&& words,
                                             std::map<std::string, std::map<int, double>, std::less<>>& word_to_document_freqs);
};

// ----------------------------------------------------------------
// Helper methods
// ----------------------------------------------------------------

/// Exceptions safety version of AddDocument
void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status = DocumentStatus::ACTUAL,
                 const std::vector<int>& ratings = {});

/// Exceptions safety version of FindTopDocuments
void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query);

/// Exceptions safety version of FindTopDocuments
void MatchDocuments(const SearchServer& search_server, const std::string& query);

/// Pagenate mathing documents
template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

// ----------------------------------------------------------------
// SearchServer template members implementation
// ----------------------------------------------------------------

template <class Container>
SearchServer::SearchServer(const Container& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words.begin(), stop_words.end())) {
    if (any_of(stop_words_.begin(), stop_words_.end(), [](const auto word) {
            return !IsValidWord(word);
        })) {
        throw std::invalid_argument("Invalid stop words"s);
    }
}

template <typename ExecutionPolicy, typename DocumentPredicate, EnableForExecutionPolicy<ExecutionPolicy>>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentPredicate predicate) const {
    auto query = ParseQuery(raw_query, false);
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

template <typename ExecutionPolicy, typename DocumentPredicate, EnableForExecutionPolicy<ExecutionPolicy>>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy, const Query& query, DocumentPredicate predicate) const {
    if (query.plus_words.empty()) {
        return {};
    }

    constexpr bool is_seq = !std::is_convertible<ExecutionPolicy, std::execution::parallel_policy>::value;
    size_t default_bucket_size = is_seq ? 1ul : std::min(word_to_document_freqs_.size(), 1000ul);

    ConcurrentSet<int> cm_exclude_doc_ids{default_bucket_size * (query.minus_words.empty() ? 0ul : 1ul)};
    std::for_each(policy, query.minus_words.begin(), query.minus_words.end(), [this, &cm_exclude_doc_ids](const std::string_view minus_word) {
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
    ConcurrentMap<int, double> cm_document_to_relevance(default_bucket_size * (query.plus_words.empty() ? 0ul : 1ul));
    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
                  [this, &cm_document_to_relevance, predicate, &exclude_doc_ids](const std::string_view word) {
                      auto ptr = word_to_document_freqs_.find(word);
                      if (ptr == word_to_document_freqs_.end()) {
                          return;
                      }

                      const auto& words = ptr->second;

                      const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                      for (const auto& [document_id, term_freq] : words) {
                          if (exclude_doc_ids.count(document_id)) {
                              continue;
                          }
                          const auto& document_data = documents_.at(document_id);
                          if (predicate(document_id, document_data.status, document_data.rating)) {
                              cm_document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
                          }
                      }
                  });

    const auto docs = cm_document_to_relevance.BuildOrdinaryVector();
    std::vector<Document> matched_documents(docs.size());
    std::transform(policy, std::make_move_iterator(docs.begin()), std::make_move_iterator(docs.end()), matched_documents.begin(),
                   [this](const auto item) -> Document {
                       return {item.first, item.second, documents_.at(item.first).rating};
                   });

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate predicate) const {
    return FindAllDocuments(std::execution::seq, query, predicate);
}

template <typename ExecutionPolicy>
void SearchServer::EraseFromWordToDocumentFreqs(ExecutionPolicy&& policy, int id, std::vector<std::string_view>&& words,
                                                std::map<std::string, std::map<int, double>, std::less<>>& word_to_document_freqs) {
    std::for_each(policy, words.begin(), words.end(), [&word_to_document_freqs, id, &policy](const std::string_view cur_word) {
        auto docs_ptr = word_to_document_freqs.find(cur_word);
        if (docs_ptr == word_to_document_freqs.end() || docs_ptr->second.empty()) {
            return;
        }
        auto& ids_freq = docs_ptr->second;
        ids_freq.erase(find_if(policy, ids_freq.begin(), ids_freq.end(), [id](const std::pair<int, double>& item) {
            return item.first == id;
        }));
    });
}

template <typename ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy>>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, const std::string_view raw_query,
                                                                                      int document_id) const {
    auto word_freqs_ptr = document_to_words_freqs_.find(document_id);
    if (word_freqs_ptr == document_to_words_freqs_.end()) {
        throw std::out_of_range("No document with id: "s + std::to_string(document_id));
    }

    const auto& words = word_freqs_ptr->second;

    Query query = ParseQuery(raw_query, false);
    std::tuple<std::vector<std::string_view>, DocumentStatus> result{{}, documents_.at(document_id).status};

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

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentPredicate predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template <class ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy>>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query, [status](int id, DocumentStatus doc_status, [[maybe_unused]] int rating) -> bool {
        return (doc_status == status);
    });
}

template <class ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy>>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <class ExecutionPolicy, EnableForExecutionPolicy<ExecutionPolicy>>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id) {
    if (document_ids_.empty() || !documents_.count(document_id)) {
        return;
    }
    auto doc_words_ptr = document_to_words_freqs_.find(document_id);
    if (doc_words_ptr == document_to_words_freqs_.end() || word_to_document_freqs_.empty()) {
        return;
    }

    std::set<std::string> exclude_words{};
    auto hash = BuildHash(doc_words_ptr->second, exclude_words);
    auto& hash_ids = hash_content_.at(hash);
    hash_ids.erase(document_id);
    if (hash_ids.empty()) {
        hash_content_.erase(hash);
    }

    std::vector<std::string_view> words{doc_words_ptr->second.size()};
    std::transform(policy, doc_words_ptr->second.begin(), doc_words_ptr->second.end(), words.begin(), [](const auto& item) {
        return item.first;
    });

    EraseFromWordToDocumentFreqs(policy, document_id, std::move(words), word_to_document_freqs_);

    document_ids_.erase(std::find(document_ids_.begin(), document_ids_.end(), document_id));
    documents_.erase(document_id);
    document_to_words_freqs_.erase(doc_words_ptr);
}