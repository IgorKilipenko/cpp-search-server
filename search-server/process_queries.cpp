#include "process_queries.h"

#include <pstl/glue_execution_defs.h>

#include <algorithm>
#include <cstddef>
#include <execution>
#include <functional>
#include <iterator>
#include <list>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "document.h"
#include "search_server.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries) {
    using Documents = std::vector<std::vector<Document>>;
    if (queries.empty()) {
        return {};
    }
    Documents documents_lists(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), documents_lists.begin(), [&search_server](const std::string& query) {
        return search_server.FindTopDocuments(query);
    });

    return documents_lists;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
    using Documents = std::vector<Document>;
    if (queries.empty()) {
        return {};
    }
    Documents documents_lists;
    documents_lists.reserve(queries.size());
    documents_lists = std::transform_reduce(
        std::execution::par, queries.begin(), queries.end(), documents_lists,
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

std::list<Document> ProcessQueriesJoinedList(const SearchServer& search_server, const std::vector<std::string>& queries) {
    using Documents = std::list<Document>;
    if (queries.empty()) {
        return {};
    }
    return std::transform_reduce(
        std::execution::par, queries.begin(), queries.end(), Documents{},
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
