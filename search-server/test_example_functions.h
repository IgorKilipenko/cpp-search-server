#pragma once
#include <execution>
#include <random>

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
    // TestParRemoveDocument();

    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    cout << search_server.GetDocumentCount() << endl;
}

#define TEST_REMOVE_DOCUMENT(mode) Test(#mode, search_server, execution::mode)

void TestParMatchDocument();

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