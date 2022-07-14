#include "process_queries.h"

#include <pstl/glue_execution_defs.h>

#include <algorithm>
#include <cstddef>
#include <execution>
#include <functional>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

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