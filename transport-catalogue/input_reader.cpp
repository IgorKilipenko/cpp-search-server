#include "input_reader.h"

namespace transport_catalogue::io::detail {
    size_t TrimStart(std::string_view& str, const char ch) {
        size_t idx = str.find_first_not_of(ch);
        if (idx != std::string::npos) {
            str.remove_prefix(idx);
            return idx;
        }
        return 0;
    }
    size_t TrimEnd(std::string_view& str, const char ch) {
        size_t idx = str.find_last_not_of(ch);
        if (idx != str.npos) {
            str.remove_suffix(str.size() - idx - 1);
            return idx;
        }
        return 0;
    }
    void Trim(std::string_view& str, const char ch) {
        TrimStart(str, ch);
        TrimEnd(str, ch);
    }
    std::vector<std::string_view> SplitIntoWords(std::string_view str, const char ch) {
        if (str.empty()) {
            return {};
        }
        std::vector<std::string_view> result;
        str.remove_prefix(std::min(str.find_first_not_of(ch), str.size()));

        do {
            int64_t space = str.find(ch, 0);
            result.push_back(space == static_cast<int64_t>(str.npos) ? str.substr(0) : str.substr(0, space));
            str.remove_prefix(std::min(str.find_first_not_of(ch, space), str.size()));
        } while (!str.empty());

        return result;
    }
}

namespace transport_catalogue::io {
    std::string Reader::ReadLine() const {
        std::string line;
        std::getline(in_stream_, line);
        return line;
    }

    std::vector<std::string> Reader::ReadLines(size_t count) const {
        std::vector<std::string> result{count};
        int idx = 0;
        for (std::string line; idx < count && std::getline(in_stream_, line); ++idx) {
            result[idx] = line;
        }
        return result;
    }

    void Reader::ReadRequest(const std::string_view raw_cmd) const {
        std::string_view cmd = raw_cmd;
        detail::Trim(cmd);
        assert(!cmd.empty());
        auto raw_req = parser_.SplitRequest(cmd);
        if (parser_.IsAddStopRequest(raw_req.command)) {
            // auto stop_result_ptr = parser_.ParseStopCmd(raw_cmd);
            auto [name, point] = parser_.ParseStopCmd(raw_req);
            catalog_db_.AddStop(Stop{static_cast<std::string>(name), std::move(point)});

        } else if (parser_.IsAddBusRequest(raw_req.command)) {
        }
    }

    void Reader::PorccessRequests() const {
        size_t n = Read<size_t>();
        auto lines = ReadLines(n);
        std::for_each(lines.begin(), lines.end(), [&](const std::string_view raw_cmd) {
            ReadRequest(raw_cmd);
        });
    }
}

namespace transport_catalogue::io {
    Parser::StopCmdResult Parser::ParseStopCmd(const RawRequest& req) const {
        assert(!req.value.empty() && !req.args.empty() && req.command == Names::STOP);
        return {req.value, ParseLatLng(req.args)};
    }

    std::shared_ptr<std::pair<std::string_view, std::string_view>> Parser::SplitKeyValue(const std::string_view str) const {
        size_t idx = str.find(':');
        if (idx == std::string::npos) {
            return nullptr;
        }
        std::string_view key = str.substr(0, idx);
        detail::TrimEnd(key);
        std::string_view value = str.substr(idx + 1);
        detail::TrimStart(value);
        std::pair<std::string_view, std::string_view> result{key, value};
        return std::make_shared<std::pair<std::string_view, std::string_view>>(std::move(result));
    }

    Parser::RawRequest Parser::SplitRequest(const std::string_view str) const {
        auto key_val_ptr = SplitKeyValue(str);
        assert(key_val_ptr != nullptr);
        auto data = detail::SplitIntoWords(key_val_ptr->first);
        assert(data.size() == 2);
        return RawRequest(data[0], data[1], key_val_ptr->second);
    }

    Coordinates Parser::ParseLatLng(std::string_view str, const char sep) const {
        auto point = detail::SplitIntoWords(str, sep);
        assert(point.size() == 2);
        return {std::stod(static_cast<std::string>(point[0])), std::stod(static_cast<std::string>(point[1]))};
    }

    bool Parser::IsRequestType(const std::string_view req, const std::string_view type) const {
        return type == req.substr(0, type.size());
    }

    bool Parser::IsAddStopRequest(const std::string_view req) const {
        return IsRequestType(req, Names::STOP);
    }
    bool Parser::IsAddBusRequest(const std::string_view req) const {
        return IsRequestType(req, Names::BUS);
    }
}