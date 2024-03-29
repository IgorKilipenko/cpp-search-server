#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "log_duration.h"
#include "process_queries.h"
#include "search_server.h"
#include "test_example_functions.h"

using namespace std;

int main() {
    {
        TestParFindTopDocuments();

        mt19937 generator;

        const auto dictionary = GenerateDictionary(generator, 1000, 10);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
        }

        const auto queries = GenerateQueries(generator, dictionary, 100, 70, 0.1);

        TEST_FIND_TOP_DOCUMENTS(seq);
        TEST_FIND_TOP_DOCUMENTS(par);
    }
    {
        TestParMatchDocument();

        mt19937 generator;

        const auto dictionary = GenerateDictionary(generator, 1000, 10);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

        const string query = GenerateQuery(generator, dictionary, 500, 0.1);

        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
        }

        TEST_MATCH_DOCUMENT(seq);
        TEST_MATCH_DOCUMENT(par);
    }
    {
        TestParRemoveDocument();

        mt19937 generator;

        const auto dictionary = GenerateDictionary(generator, 10'000, 25);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 100);

        {
            SearchServer search_server(dictionary[0]);
            for (size_t i = 0; i < documents.size(); ++i) {
                search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
            }

            TEST_REMOVE_DOCUMENT(seq);
        }
        {
            SearchServer search_server(dictionary[0]);
            for (size_t i = 0; i < documents.size(); ++i) {
                search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
            }

            TEST_REMOVE_DOCUMENT(par);
        }
    }
}