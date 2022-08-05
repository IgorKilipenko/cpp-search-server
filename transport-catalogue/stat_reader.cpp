
#include "stat_reader.h"

namespace transport_catalogue::io {
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

        assert(raw_req.command == Parser::Names::BUS);
        StatReader::PrintBusInfo(catalog_db_, raw_req.value);
    }

    void StatReader::PorccessRequests() const {
        PorccessGetRequests(reader_.Read<size_t>());
    }
}