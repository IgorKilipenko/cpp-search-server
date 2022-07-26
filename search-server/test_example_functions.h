#pragma once
#include <cassert>
#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "log_duration.h"
#include "search_server.h"

using namespace std;

string GenerateWord(mt19937& generator, int max_length);

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length);

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0);

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count);

void TestParRemoveDocument();

template <typename ExecutionPolicy>
void TestParRemoveDocument(string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    cout << search_server.GetDocumentCount() << endl;
}

#define TEST_REMOVE_DOCUMENT(mode) TestParRemoveDocument(#mode, search_server, execution::mode)

void TestParMatchDocument();

void TestFindTopDocuments();

template <typename ExecutionPolicy>
void TestParMatchDocument(string_view mark, SearchServer search_server, const string& query, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    int word_count = 0;
    for (int id = 0; id < document_count; ++id) {
        const auto [words, status] = search_server.MatchDocument(policy, query, id);
        word_count += words.size();
    }
    cout << word_count << endl;
}

#define TEST_MATCH_DOCUMENT(policy) TestParMatchDocument(#policy, search_server, query, execution::policy)

inline void TestToString() {
    cerr << "Start TestToString." << endl;
    string str{"test string"};
    string_view view{str};
    view = view.substr(5);
    assert(str.substr(5) == ToString(view));
    assert(str.substr(5) == static_cast<string>(view));
    assert(str.substr(5) == string(view));
    assert(str.substr(5) == view);
    cerr << "Done." << endl << endl;
}

template <typename ExecutionPolicy>
void TestParFindTopDocuments(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    double total_relevance = 0;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
        }
    }
    cout << total_relevance << endl;
}

#define TEST_FIND_TOP_DOCUMENTS(policy) TestParFindTopDocuments(#policy, search_server, queries, execution::policy)