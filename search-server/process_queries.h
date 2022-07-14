#pragma once

#include <execution>
#include <list>
#include <string>
#include <vector>
#include <algorithm>

#include "document.h"
#include "search_server.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries);

template <class ExecutionPolicy>
std::vector<std::vector<Document>> ProcessQueries(ExecutionPolicy&& policy, const SearchServer& search_server,
                                                  const std::vector<std::string>& queries);

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries);

template <class ExecutionPolicy>
std::vector<Document> ProcessQueriesJoined(ExecutionPolicy&& policy, const SearchServer& search_server, const std::vector<std::string>& queries);

std::list<Document> ProcessQueriesJoinedList(const SearchServer& search_server, const std::vector<std::string>& queries);

template <class ExecutionPolicy>
std::list<Document> ProcessQueriesJoinedList(ExecutionPolicy&& policy, const SearchServer& search_server, const std::vector<std::string>& queries);

template <class ExecutionPolicy>
std::vector<std::vector<Document>> ProcessQueries(ExecutionPolicy&& policy, const SearchServer& search_server,
                                                  const std::vector<std::string>& queries) {
    using Documents = std::vector<std::vector<Document>>;
    if (queries.empty()) {
        return {};
    }
    Documents documents_lists(queries.size());
    std::transform(policy, queries.begin(), queries.end(), documents_lists.begin(), [&search_server](const std::string& query) {
        return search_server.FindTopDocuments(query);
    });

    return documents_lists;
}

template <class ExecutionPolicy>
std::vector<Document> ProcessQueriesJoined(ExecutionPolicy&& policy, const SearchServer& search_server, const std::vector<std::string>& queries) {
    using Documents = std::vector<Document>;
    if (queries.empty()) {
        return {};
    }
    Documents documents_lists;
    documents_lists.reserve(queries.size());
    documents_lists = std::transform_reduce(
        policy, queries.begin(), queries.end(), documents_lists,
        [](Documents docs, Documents found_docs) {
            for (auto& doc : found_docs) {
                docs.push_back(std::move(doc));
            }
            return docs;
        },
        [&search_server](const std::string& query) {
            return search_server.FindTopDocuments(query);
        });

    return documents_lists;
}

template <class ExecutionPolicy>
std::list<Document> ProcessQueriesJoinedList(ExecutionPolicy&& policy, const SearchServer& search_server,
                                             const std::vector<std::string>& queries) {
    using Documents = std::list<Document>;
    if (queries.empty()) {
        return {};
    }
    return std::transform_reduce(
        policy, queries.begin(), queries.end(), Documents{},
        [](Documents docs, Documents found_docs) {
            for (auto& doc : found_docs) {
                docs.push_back(std::move(doc));
            }
            return docs;
        },
        [&search_server](const std::string& query) {
            auto docs = search_server.FindTopDocuments(query);
            return Documents(std::make_move_iterator(docs.begin()), std::make_move_iterator(docs.end()));
        });
}
