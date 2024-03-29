#include "document.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

extern void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

extern void PrintMatchDocumentResult(int document_id, const vector<string_view>& words, DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const auto word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}
