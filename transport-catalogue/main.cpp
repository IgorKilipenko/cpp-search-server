#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

namespace transport_catalogue::tests {
    using namespace transport_catalogue;
    void Test1() {
        std::stringstream ss;
        {
            std::vector<std::string> input_add_queries{
                "Stop Tolstopaltsevo: 55.611087, 37.20829",
                "Stop Marushkino: 55.595884, 37.209755",
                "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye",
                "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka",
                "Stop Rasskazovka: 55.632761, 37.333324",
                "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517",
                "Stop Biryusinka: 55.581065, 37.64839",
                "Stop Universam: 55.587655, 37.645687",
                "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656",
                "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164",
                "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye",
                "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757",
                "Stop Prazhskaya: 55.611678, 37.603831",
            };

            std::vector<std::string> input_get_queries{
                "Bus 256", "Bus 750", "Bus 751", "Stop Samara", "Stop Prazhskaya", "Stop Biryulyovo Zapadnoye",
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
        TransportCatalogue catalog;
        const io::Reader reader(*catalog.GetDatabaseForWrite(), ss);
        const io::StatReader stat_reader(*catalog.GetDatabaseForWrite(), reader);

        reader.PorccessRequests();
        stat_reader.PorccessRequests();
    }
}

int main() {
    using namespace transport_catalogue;

    tests::Test1();

    TransportCatalogue catalog;
    const io::Reader reader(*catalog.GetDatabaseForWrite(), std::cin);
    const io::StatReader stat_reader(*catalog.GetDatabaseForWrite(), reader);

    reader.PorccessRequests();
    stat_reader.PorccessRequests();

    return 0;
}