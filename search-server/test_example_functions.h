#pragma once
#include <execution>
#include <random>

#include "log_duration.h"
#include "search_server.h"

using namespace std;

string GenerateWord(mt19937& generator, int max_length);

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length);

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count);

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count);

void TestParRemoveDocument();

template <typename ExecutionPolicy>
void Test(string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
    //TestParRemoveDocument();

    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    cout << search_server.GetDocumentCount() << endl;
}

#define TEST_REMOVE_DOCUMENT(mode) Test(#mode, search_server, execution::mode)