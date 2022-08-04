#include <istream>
#include <memory>
#include <sstream>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

// using namespace std;
using namespace transport_catalogue;

int main() {
    std::cout << "Start..." << std::endl;
    //auto db = std::make_shared<TransportCatalogue::Database>(TransportCatalogue::Database());
    std::stringstream ss;
    {
        ss << "2" << '\n'; 
        ss << "Stop Tolstopaltsevo: 55.611087, 37.208290" << '\n';
        ss << "Stop Marushkino: 55.595884, 37.209755" << std::endl;
    }
    std::shared_ptr<TransportCatalogue::Database> db = nullptr;
    {
        TransportCatalogue catalog;
        io::Reader reader(*catalog.GetDatabase(), ss);

        reader.PorccessRequests();

        db = catalog.GetDatabase();
    }

    return 0;
}