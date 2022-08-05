#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;

int main() {
    TransportCatalogue catalog;
    const io::Reader reader(*catalog.GetDatabaseForWrite(), std::cin);
    const io::StatReader stat_reader(*catalog.GetDatabaseForWrite(), reader);

    reader.PorccessRequests();
    stat_reader.PorccessRequests();

    return 0;
}