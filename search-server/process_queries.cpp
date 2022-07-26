#include "process_queries.h"
#include <pstl/glue_execution_defs.h>

#include <execution>
#include <list>
#include <string>
#include <vector>

#include "document.h"
#include "search_server.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries) {
    return ProcessQueries(std::execution::par, search_server, queries);
}

std::list<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
    return ProcessQueriesJoined(std::execution::par, search_server, queries);
}