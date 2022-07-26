#pragma once

#include <algorithm>
#include <execution>
#include <iterator>
#include <list>
#include <string>
#include <vector>

#include "document.h"
#include "search_server.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries);

std::list<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries);

template <typename ExecutionPolicy>
std::vector<std::vector<Document>> ProcessQueries(ExecutionPolicy&& policy, const SearchServer& search_server,
                                                  const std::vector<std::string>& queries) {
    using Documents = std::vector<std::vector<Document>>;
    if (queries.empty()) {
        return {};
    }
    Documents documents_lists(queries.size());
    std::transform(policy, queries.begin(), queries.end(), documents_lists.begin(), [&search_server](const std::string& query) {
        return std::move(search_server.FindTopDocuments(query));
    });

    return documents_lists;
}

template <typename ExecutionPolicy=std::execution::parallel_policy, template<typename, typename...> typename Container = std::list, typename TDoc = Document> 
Container<TDoc> ProcessQueriesJoined(ExecutionPolicy&& policy, const SearchServer& search_server, const std::vector<std::string>& queries) {
    using Documents = Container<TDoc>;
    if (queries.empty()) {
        return {};
    }
    return std::transform_reduce(
        policy, queries.begin(), queries.end(), Documents{},
        [](Documents docs, Documents found_docs) {
            for (auto& doc : found_docs) {
                docs.push_back(std::move(doc));
            }
            // docs.splice(docs.end(), std::move(found_docs));
            return docs;
        },
        [&search_server](const std::string& query) {
            auto docs = search_server.FindTopDocuments(query);
            return Documents(std::make_move_iterator(docs.begin()), std::make_move_iterator(docs.end()));
        });
}
