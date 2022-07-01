#pragma once

#include <ostream>

using namespace std;

struct Document {
    int id;
    double relevance;
    int rating;
    Document() : id{0}, relevance{0.0}, rating{0} {}
    Document(int id, double relevance, int rating) : id{id}, relevance{relevance}, rating{rating} {}
    friend ostream& operator<<(ostream& os, const Document& doc) {
        os << "{ document_id = " << doc.id << " relevance = " << doc.relevance << " rating = " << doc.rating << " }";
        return os;
    }
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};