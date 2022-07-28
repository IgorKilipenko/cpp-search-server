#pragma once
#include <cassert>
#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "log_duration.h"
#include "search_server.h"

std::string GenerateWord(std::mt19937& generator, int max_length);

std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length);

std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int word_count, double minus_prob = 0);

std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count,
                                         double minus_prob = 0);

void TestParRemoveDocument();

template <typename ExecutionPolicy>
void TestParRemoveDocument(std::string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    std::cout << search_server.GetDocumentCount() << std::endl;
}

#define TEST_REMOVE_DOCUMENT(mode) TestParRemoveDocument(#mode, search_server, execution::mode)

void TestParMatchDocument();

void TestParFindTopDocuments();

template <typename ExecutionPolicy>
void TestParMatchDocument(std::string_view mark, SearchServer search_server, const std::string& query, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    int word_count = 0;
    for (int id = 0; id < document_count; ++id) {
        const auto [words, status] = search_server.MatchDocument(policy, query, id);
        word_count += words.size();
    }
    std::cout << word_count << std::endl;
}

#define TEST_MATCH_DOCUMENT(policy) TestParMatchDocument(#policy, search_server, query, execution::policy)

inline void TestToString() {
    std::cerr << "Start TestToString." << std::endl;
    std::string str{"test string"};
    std::string_view view{str};
    view = view.substr(5);
    assert(str.substr(5) == ToString(view));
    assert(str.substr(5) == static_cast<string>(view));
    assert(str.substr(5) == string(view));
    assert(str.substr(5) == view);
    std::cerr << "Done." << std::endl << std::endl;
}

template <typename ExecutionPolicy>
void TestParFindTopDocuments(std::string_view mark, const SearchServer& search_server, const std::vector<std::string>& queries,
                             ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    double total_relevance = 0;
    for (const std::string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
        }
    }
    std::cout << total_relevance << std::endl;
}

#define TEST_FIND_TOP_DOCUMENTS(policy) TestParFindTopDocuments(#policy, search_server, queries, execution::policy)