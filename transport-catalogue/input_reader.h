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
        std::shared_ptr<StopCmdResult> ParseStopCmd(const std::string_view raw_cmd) const {
            if (raw_cmd.empty()) {
                return nullptr;
            }
            std::string_view cmd = raw_cmd;
            // Trim(cmd);
            assert(IsStopCmd(cmd));

            cmd.remove_prefix(Names::STOP.size());
            detail::TrimStart(cmd);

            std::cout << "IS STOP command" << std::endl;

            auto key_val_ptr = SplitKeyValue(cmd);
            if (key_val_ptr) {
                // stops_.emplace(key_val_ptr->first, ParseLatLng(key_val_ptr->second));
                StopCmdResult result{key_val_ptr->first, ParseLatLng(key_val_ptr->second)};
                return std::make_shared<StopCmdResult>(std::move(result));
            }

            return nullptr;
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
            auto strings = detail::SplitIntoWords(key_val_ptr->first);
            assert(strings.size() == 2);
            return RawRequest(strings[0], strings[1], key_val_ptr->second);
        }

        Coordinates ParseLatLng(std::string_view str) const {
            return {};
        }

        bool IsStopCmd(const std::string_view cmd) const {
            return Names::STOP == cmd.substr(0, Names::STOP.size());
        }
        const StopsContainer& GetStops() const {
            return stops_;
        }

    private:
        StopsContainer stops_;
    };

    class Reader {
    public:
        struct Request {
            enum class RequestType { ADD, REMOVE };
            RequestType type = RequestType::ADD;
            Stop value;
        };
        Reader(TransportCatalogue& catalog, std::istream& in_stream = std::cin) : in_stream_{in_stream}, catalog_{catalog} {}
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
        void PraseRequest(const std::string_view raw_cmd) {
            std::string_view cmd = raw_cmd;
            detail::Trim(cmd);
            assert(!cmd.empty());
            auto raw_req = parser_.SplitRequest(cmd);
            if (parser_.IsStopCmd(raw_req.command)) {
                auto stop_result_ptr = parser_.ParseStopCmd(raw_cmd);
                catalog_.AddStop(static_cast<std::string>(stop_result_ptr->first), std::move(stop_result_ptr->second));
            }
        }
        void PorccessRequests() {
            size_t n = Read<size_t>();
            auto lines = ReadLines(n);
            std::for_each(lines.begin(), lines.end(), [&](const std::string_view raw_cmd) {
                PraseRequest(raw_cmd);
            });
        }

    private:
        std::istream& in_stream_;
        Parser parser_;
        TransportCatalogue catalog_;
    };
}