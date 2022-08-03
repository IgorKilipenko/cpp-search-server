#include <iostream>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

// using namespace std;
using namespace transport_catalogue;

int main() {
    std::cout << "Start..." << std::endl;
    io::Reader reader(std::cin);
    TransportCatalogue catalog;
    std::string cur_line;
    while (!(cur_line = reader.ReadRawLine()).empty()) {
        std::cout << "> " << cur_line << std::endl;
        reader.PraseCommand(cur_line);
    }

    
    catalog.AddStop("TestStop", geo::Coordinates{99.55, 88.55});
    geo::Coordinates coords{99.55, 88.55};
    catalog.AddStop("TestStop2", std::move(coords));
    const geo::Coordinates coords2{89.55, 88.55};
    catalog.AddStop("TestStop3", coords2);

    return 0;
}