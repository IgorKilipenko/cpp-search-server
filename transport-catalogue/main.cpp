#include <algorithm>
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
    std::cout << "Start..." << std::endl;
    // auto db = std::make_shared<TransportCatalogue::Database>(TransportCatalogue::Database());
    std::stringstream ss;
    {
        std::vector<std::string> input_lines{
            "Stop Tolstopaltsevo: 55.611087, 37.208290", 
            "Stop Marushkino: 55.595884, 37.209755",
            //"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye",
            //"Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"
        };
        ss << input_lines.size() << '\n';
        std::for_each(input_lines.begin(), input_lines.end(), [&](const std::string_view line) {
            ss << line << '\n';
        });
    }
    std::shared_ptr<TransportCatalogue::Database> db = nullptr;
    {
        TransportCatalogue catalog;
        const io::Reader reader(*catalog.GetDatabase(), ss);

        reader.PorccessRequests();

        db = catalog.GetDatabase();
    }

    return 0;
}