#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace transport_catalogue::io::detail {
    using namespace std::literals;

    size_t TrimStart(std::string_view& str, const char ch = ' ');
    size_t TrimEnd(std::string_view& str, const char ch = ' ');
    void Trim(std::string_view& str, const char ch = ' ');
    std::vector<std::string_view> SplitIntoWords(std::string_view str, const char ch = ' ');

    class CommandParser {
        using Coordinates = geo::Coordinates;
        using StopsContainer = std::unordered_map<std::string_view, Coordinates>;

    public:
        struct Names {
            static constexpr const std::string_view STOP = "Stop"sv;
            static constexpr const std::string_view BUS = "Bus"sv;
        };
        bool Parse(const std::string_view raw_cmd) {
            if (raw_cmd.empty()) {
                return false;
            }
            std::string_view cmd = raw_cmd;
            Trim(cmd);
            if (IsAddStopCmd(cmd)) {
                cmd.remove_prefix(Names::STOP.size());
                TrimStart(cmd);
                std::cout << "IS STOP command" << std::endl;
                auto key_val_ptr = SplitKeyValue(cmd);
                if (key_val_ptr) {
                    stops_.emplace(key_val_ptr->first, ParseLatLng(key_val_ptr->second)); 
                    return true;
                }
            }
            return false;
        }
        static std::shared_ptr<std::pair<std::string_view, std::string_view>> SplitKeyValue(const std::string_view str) {
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
        static Coordinates ParseLatLng(std::string_view str) {
            return {};
        }

        static bool IsAddStopCmd(const std::string_view cmd) {
            return Names::STOP == cmd.substr(0, Names::STOP.size());
        }

        const StopsContainer& GetStops() const {
            return stops_;
        }

    private:
        StopsContainer stops_;
    };
}

namespace transport_catalogue::io {
    class Reader {
    public:
        Reader(std::istream& in_stream = std::cin) : in_stream_{in_stream} {}
        template <typename TOut = std::string>
        TOut Read();
        std::string ReadRawLine();
        std::vector<std::string> ReadAllInputLines();
        void PraseCommand(const std::string_view raw_cmd) const {
            cmd_parser_.Parse(raw_cmd);
        }

    private:
        std::istream& in_stream_;
        io::detail::CommandParser cmd_parser_;
    };
}