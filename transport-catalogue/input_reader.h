#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue::io::detail {
    size_t TrimStart(std::string_view& str, const char ch = ' ');
    size_t TrimEnd(std::string_view& str, const char ch = ' ');
    void Trim(std::string_view& str, const char ch = ' ');
    std::vector<std::string_view> SplitIntoWords(std::string_view str, const char ch = ' ');
}

namespace transport_catalogue::io {
    using namespace std::literals;

    class Parser {
        using Coordinates = geo::Coordinates;
        using StopsContainer = std::unordered_map<std::string_view, Coordinates>;

    public:
        using StopCmdResult = std::pair<std::string_view, Coordinates>;
        struct RawRequest {
            std::string_view command;
            std::string_view value;
            std::string_view args;

            RawRequest() = default;
            RawRequest(std::string_view command, std::string_view value, std::string_view args) : command{command}, value{value}, args{args} {}
        };
        struct Names {
            static constexpr const std::string_view STOP = "Stop"sv;
            static constexpr const std::string_view BUS = "Bus"sv;
        };
        StopCmdResult ParseStopCmd(const RawRequest& req) const {
            assert (!req.value.empty() && !req.args.empty() && req.command == Names::STOP);
            return {req.value, ParseLatLng(req.args)};
        }

        std::shared_ptr<std::pair<std::string_view, std::string_view>> SplitKeyValue(const std::string_view str) const {
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

        RawRequest SplitRequest(const std::string_view str) const {
            auto key_val_ptr = SplitKeyValue(str);
            assert(key_val_ptr != nullptr);
            auto data = detail::SplitIntoWords(key_val_ptr->first);
            assert(data.size() == 2);
            return RawRequest(data[0], data[1], key_val_ptr->second);
        }

        Coordinates ParseLatLng(std::string_view str, const char sep = ',') const {
            auto point = detail::SplitIntoWords(str, sep);
            assert(point.size() == 2);
            return {std::stod(static_cast<std::string>(point[0])), std::stod(static_cast<std::string>(point[1]))};
        }
        bool IsRequestType(const std::string_view req, const std::string_view type) const {
            return type == req.substr(0, type.size());
        }
        bool IsAddStopRequest(const std::string_view req) const {
            return IsRequestType(req, Names::STOP);
        }
        bool IsAddBusRequest(const std::string_view req) const {
            return IsRequestType(req, Names::BUS);
        }
    };

    class Reader {
    public:
        struct Request {
            enum class RequestType { ADD, REMOVE };
            RequestType type = RequestType::ADD;
            Stop value;
        };
        Reader(TransportCatalogue::Database& db, std::istream& in_stream = std::cin) : in_stream_{in_stream}, catalog_db_{db} {}
        template <typename TOut = std::string>
        TOut Read() {
            TOut result;
            in_stream_ >> result;
            in_stream_.get();
            return result;
        }
        std::string ReadRawLine();
        std::vector<std::string> ReadLines(size_t count) {
            std::vector<std::string> result{count};
            int idx = 0;
            for (std::string line; idx < count && std::getline(in_stream_, line); ++idx) {
                result[idx] = line;
            }
            return result;
        }
        void ReadRequest(const std::string_view raw_cmd) {
            std::string_view cmd = raw_cmd;
            detail::Trim(cmd);
            assert(!cmd.empty());
            auto raw_req = parser_.SplitRequest(cmd);
            if (parser_.IsAddStopRequest(raw_req.command)) {
                //auto stop_result_ptr = parser_.ParseStopCmd(raw_cmd);
                auto [name, point] = parser_.ParseStopCmd(raw_req);
                catalog_db_.AddStop(Stop{static_cast<std::string>(name), std::move(point)});

            } else if (parser_.IsAddBusRequest(raw_req.command)) {
                
            }
        }

        void PorccessRequests() {
            size_t n = Read<size_t>();
            auto lines = ReadLines(n);
            std::for_each(lines.begin(), lines.end(), [&](const std::string_view raw_cmd) {
                ReadRequest(raw_cmd);
            });
        }

    private:
        std::istream& in_stream_;
        Parser parser_;
        TransportCatalogue::Database& catalog_db_;
    };
}