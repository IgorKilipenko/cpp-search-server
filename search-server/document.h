#pragma once

#include <ostream>
#include <string>
#include <vector>

struct Document {
    int id = 0;

    double relevance = 0.0;

    int rating = 0;

    Document() : id{0}, relevance{0.0}, rating{0} {}

    Document(int id, double relevance, int rating) : id{id}, relevance{relevance}, rating{rating} {}

    friend std::ostream& operator<<(std::ostream& out, const Document& document) {
        using namespace std::string_literals;

        out << "{ "s
            << "document_id = "s << document.id << ", "s
            << "relevance = "s << document.relevance << ", "s
            << "rating = "s << document.rating << " }"s;
        return out;
    }
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

inline std::ostream& operator<<(std::ostream& out, const DocumentStatus& status) {
    std::string result;
    switch (status) {
        case DocumentStatus::ACTUAL:
            result = "ACTUAL";
            break;
        case DocumentStatus::IRRELEVANT:
            result = "IRRELEVANT";
            break;
        case DocumentStatus::BANNED:
            result = "BANNED";
            break;
        case DocumentStatus::REMOVED:
            result = "REMOVED";
            break;
    }
    out << result;
    return out;
}

void PrintDocument(const Document& document);

void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view>& words, DocumentStatus status);
