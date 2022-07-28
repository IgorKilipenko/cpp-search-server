#include "search_server.h"

/// Remove all documents duplicates from database
inline void RemoveDuplicates(SearchServer& search_server) {
    search_server.RemoveDuplicates();
}