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
                "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino",
                "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino",
                "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye",
                "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka",
                "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino",
                "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam",
                "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam",
                "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya",
                "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya",
                "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye",
                "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye",
                "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757",
                "Stop Prazhskaya: 55.611678, 37.603831",
            };

            std::vector<std::string> input_get_queries{
                "Bus 256", "Bus 750", "Bus 751", "Stop Samara", "Stop Prazhskaya", "Stop Biryulyovo Zapadnoye ",
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