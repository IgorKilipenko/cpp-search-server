#include <iostream>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

// using namespace std;
using namespace transport_catalogue;

int main() {
    std::cout << "Start..." << std::endl;
    TransportCatalogue catalog;
    io::Reader reader(catalog, std::cin);
    
    reader.PorccessRequests();

    return 0;
}