#include <algorithm>
#include <iostream>
#include <istream>
#include <memory>
#include <sstream>
#include <string_view>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

// using namespace std;
using namespace transport_catalogue;

int main() {
    std::shared_ptr<TransportCatalogue::Database> db_ptr = nullptr;
    {
        TransportCatalogue catalog;
        const io::Reader reader(*catalog.GetDatabaseForWrite(), std::cin);

        reader.PorccessRequests();

        /*db_ptr = catalog.GetDatabaseForWrite();
        auto& db = *db_ptr;
        [[maybe_unused]] const auto& stop = db.GetStopsTable();
        [[maybe_unused]] const auto& bus = db.GetBusRoutesTable();*/
    }

    return 0;
}