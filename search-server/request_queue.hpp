#pragma once
#include <chrono>
#include <functional>
#include <queue>
#include <string>

#include "server.hpp"

class RequestQueue {
   public:
    explicit RequestQueue(const SearchServer& search_server) : server_{search_server}, server_time_{0} {}

    template <typename DocumentPredicate = function<bool(int, DocumentStatus, int)>>
    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
        if (server_time_ < min_in_day_) {
            server_time_ += chrono::minutes(1);
        } else {
            requests_.pop_front();
        }

        const auto& found_docs = server_.FindTopDocuments(raw_query, document_predicate);

        requests_.push_back(QueryResult{server_time_, found_docs.size(), raw_query});
        return found_docs;
    }

    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus status) {
        return AddFindRequest(raw_query, [status]([[maybe_unused]] int id, DocumentStatus doc_status, [[maybe_unused]] int rating) {
            return doc_status == status;
        });
    }

    vector<Document> AddFindRequest(const string& raw_query) {
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
        chrono::minutes time;
        size_t response_documents_count;
        string raw_query;

        QueryResult(chrono::minutes time, size_t response_documents_count, string raw_query)
            : time{time}, response_documents_count{response_documents_count}, raw_query{raw_query} {}
    };
    deque<QueryResult> requests_;
    constexpr const static chrono::minutes min_in_day_{chrono::hours(24)};
    const SearchServer& server_;
    chrono::minutes server_time_;

    static bool IsEmptyResult(const vector<Document>& docs) {
        return docs.empty();
    }
};