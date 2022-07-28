#pragma once
#include <chrono>
#include <functional>
#include <queue>
#include <string>

#include "search_server.h"

class RequestQueue {
   public:
    explicit RequestQueue(const SearchServer& search_server) : server_{search_server}, server_time_{0} {}

    template <typename DocumentPredicate = std::function<bool(int, DocumentStatus, int)>>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        if (server_time_ < min_in_day_) {
            server_time_ += std::chrono::minutes(1);
        } else {
            requests_.pop_front();
        }

        const auto& found_docs = server_.FindTopDocuments(raw_query, document_predicate);

        requests_.push_back(QueryResult{server_time_, found_docs.size(), raw_query});
        return found_docs;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        return AddFindRequest(raw_query, [status]([[maybe_unused]] int id, DocumentStatus doc_status, [[maybe_unused]] int rating) {
            return doc_status == status;
        });
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }

    int GetNoResultRequests() const {
        int result = count_if(requests_.begin(), requests_.end(), [](QueryResult res) {
            return res.response_documents_count == 0;
        });
        return result;
    }

   private:
    struct QueryResult {
        std::chrono::minutes time;
        size_t response_documents_count;
        std::string raw_query;

        QueryResult(std::chrono::minutes time, size_t response_documents_count, std::string raw_query)
            : time{time}, response_documents_count{response_documents_count}, raw_query{raw_query} {}
    };
    std::deque<QueryResult> requests_;
    constexpr const static std::chrono::minutes min_in_day_{std::chrono::hours(24)};
    const SearchServer& server_;
    std::chrono::minutes server_time_;
};
