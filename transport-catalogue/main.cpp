#include <iostream>
#include <memory>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

// using namespace std;
using namespace transport_catalogue;

int main() {
    std::cout << "Start..." << std::endl;
    //auto db = std::make_shared<TransportCatalogue::Database>(TransportCatalogue::Database());
    std::shared_ptr<TransportCatalogue::Database> db = nullptr;
    {
        TransportCatalogue catalog;
        io::Reader reader(*catalog.GetDatabase(), std::cin);

        reader.PorccessRequests();

        db = catalog.GetDatabase();
    }

    return 0;
}