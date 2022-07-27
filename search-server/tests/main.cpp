#include <iostream>
#include <string>

#include "../search_server.h"
#include "server_test.h"

using namespace tests;

int main() {
    TestSearchServer();
    cout << "Search server testing finished"s << endl;

    return 0;
}
