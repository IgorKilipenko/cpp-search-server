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
    //std::cout << "Start..." << std::endl;
    // auto db = std::make_shared<TransportCatalogue::Database>(TransportCatalogue::Database());
    std::stringstream ss;
    {
        std::vector<std::string> input_add_queries{
            "Stop Tolstopaltsevo:  55.611087  ,     37.208290   ",
            "Stop Marushkino: 55.595884, 37.209755",
            "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye",
            "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka",
            "Stop Rasskazovka: 55.632761, 37.333324",
            "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700",
            "Stop Biryusinka: 55.581065, 37.648390",
            "Stop Universam: 55.587655, 37.645687",
            "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656",
            "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164",
        };

        std::vector<std::string> input_get_queries{
            "Bus 256",
            "Bus 750",
            "Bus 751",
        };

        ss << input_add_queries.size() << '\n';
        std::for_each(input_add_queries.begin(), input_add_queries.end(), [&](const std::string_view line) {
            ss << line << '\n';
        });

        ss << input_get_queries.size() << '\n';
        std::for_each(input_get_queries.begin(), input_get_queries.end(), [&](const std::string_view line) {
            ss << line << '\n';
        });
    }

    std::shared_ptr<TransportCatalogue::Database> db_ptr = nullptr;
    {
        TransportCatalogue catalog;
        const io::Reader reader(*catalog.GetDatabaseForWrite(), ss);

        reader.PorccessRequests();

        db_ptr = catalog.GetDatabaseForWrite();
        auto& db = *db_ptr;
        [[maybe_unused]] const auto& stop = db.GetStopsTable();
        [[maybe_unused]] const auto& bus = db.GetBusRoutesTable();
    }

    return 0;
}