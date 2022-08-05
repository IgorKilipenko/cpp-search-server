#pragma once

#include <iomanip>
#include <iostream>

#include "input_reader.h"
#include "transport_catalogue.h"

namespace transport_catalogue::io {
    class StatReader {
    public:
        StatReader(TransportCatalogue::Database &catalog_db, const Reader &reader) : reader_{reader}, catalog_db_{catalog_db} {}

        void PrintBusInfo(const std::string_view bus_name) const;

        void PrintStopInfo(const std::string_view stop_name) const;

        void PorccessGetRequests(size_t n) const;

        void PorccessRequests() const;

        void ExecuteRequest(const Parser::RawRequest &raw_req) const;

    private:
        const Reader &reader_;
        TransportCatalogue::Database &catalog_db_;
    };

}
