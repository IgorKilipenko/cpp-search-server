#include <string_view>

#include "input_reader.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;

int main() {
    TransportCatalogue catalog;
    const io::Reader reader(*catalog.GetDatabaseForWrite(), std::cin);
    reader.PorccessRequests();

    return 0;
}