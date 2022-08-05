
#include "stat_reader.h"

namespace transport_catalogue::io {
            void StatReader::PrintBusInfo(const TransportCatalogue::Database &db, const std::string_view bus_name) {
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

        void StatReader::PrintStopInfo(const TransportCatalogue::Database &db, const std::string_view stop_name) {
            using namespace std::string_view_literals;

            std::cout << "Stop "sv << stop_name << ": "sv;
            
            const Stop* stop = db.GetStop(stop_name);
            if (stop == nullptr) {
                std::cout << "not found"sv << std::endl;
                return;
            }

            const auto &buses_names = db.GetBuses(stop);
            if (buses_names.empty()) {
                std::cout << "no buses"sv << std::endl;
                return;
            }

            std::cout << "buses"sv;
            for (const auto &bus : buses_names) {
                std::cout << " "sv << bus;
            }
            std::cout << std::endl;
        }

    void StatReader::PorccessGetRequests(size_t n) const {
        auto lines = reader_.ReadLines(n);
        const Parser& parser_ = reader_.GetParser();
        std::for_each(std::make_move_iterator(lines.begin()), std::make_move_iterator(lines.end()), [this, &parser_](const std::string_view str) {
            if (parser_.IsGetRequest(str)) {
                auto raw_req = parser_.SplitRequest(str);
                ExecuteRequest(raw_req);
            }
        });
    }

    void StatReader::ExecuteRequest(const Parser::RawRequest& raw_req) const {
        assert(raw_req.type == Parser::RawRequest::Type::GET);
        assert(!raw_req.value.empty() && !raw_req.command.empty() && raw_req.args.empty());
        assert((raw_req.command == Parser::Names::BUS) || (raw_req.command == Parser::Names::STOP));

        if (raw_req.command == Parser::Names::BUS) {
            StatReader::PrintBusInfo(catalog_db_, raw_req.value);
        }
        if (raw_req.command == Parser::Names::STOP) {
            StatReader::PrintStopInfo(catalog_db_, raw_req.value);
        }
    }

    void StatReader::PorccessRequests() const {
        PorccessGetRequests(reader_.Read<size_t>());
    }
}