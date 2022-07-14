#include "process_queries.h"
#include "search_server.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const vector<string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
        cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
    }

    return 0;
}

/* 
Expected results:
Document 1 matched with relevance 0.183492
Document 5 matched with relevance 0.183492
Document 4 matched with relevance 0.167358
Document 3 matched with relevance 0.743945
Document 1 matched with relevance 0.311199
Document 2 matched with relevance 0.183492
Document 5 matched with relevance 0.127706
Document 4 matched with relevance 0.0557859
Document 2 matched with relevance 0.458145
Document 5 matched with relevance 0.458145 
*/