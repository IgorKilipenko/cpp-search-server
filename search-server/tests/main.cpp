#include <iostream>
#include <string>

#include "../search_server.h"
#include "server_test.h"

using namespace tests;
using namespace std;

int main() {
    TestSearchServer();
    cout << endl << "Search server testing finished"s << endl << endl;

    return 0;
}
