#pragma once

#include <iomanip>
#include <iostream>

#include "input_reader.h"
#include "transport_catalogue.h"

namespace transport_catalogue::io {
    class StatReader {
    public:
        StatReader(TransportCatalogue::Database &catalog_db, const Reader &reader) : reader_{reader}, catalog_db_{catalog_db} {}
        static void PrintBusInfo(const TransportCatalogue::Database &db, const std::string_view bus_name) {
            using namespace std::string_view_literals;

            const data::Bus *bus = db.GetBus(bus_name);
            std::cout << "Bus "sv << bus_name << ": "sv;

            if (bus == nullptr) {
                std::cout << "not found"sv << std::endl;
            } else {
                auto info = db.GetBusInfo(bus);
                std::cout << info.total_stops << " stops on route, "sv << info.unique_stops << " unique stops, "sv << std::setprecision(6)
                          << info.route_length << " route length"sv << std::endl;
            }
        }

        void PorccessGetRequests(size_t n) const;

        void PorccessRequests() const;

        void ExecuteRequest(const Parser::RawRequest &raw_req) const;

    private:
        const Reader& reader_;
        TransportCatalogue::Database &catalog_db_;
    };

}
