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
        StopCmdResult ParseStopCmd(const RawRequest& req) const;

        std::shared_ptr<std::pair<std::string_view, std::string_view>> SplitKeyValue(const std::string_view str) const;

        RawRequest SplitRequest(const std::string_view str) const;

        Coordinates ParseLatLng(std::string_view str, const char sep = ',') const;

        bool IsRequestType(const std::string_view req, const std::string_view type) const;

        bool IsAddStopRequest(const std::string_view req) const;

        bool IsAddBusRequest(const std::string_view req) const;
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
        TOut Read() const;

        std::string ReadLine() const;

        std::vector<std::string> ReadLines(size_t count) const;

        void ReadRequest(const std::string_view raw_cmd) const;

        void PorccessRequests() const;

    private:
        std::istream& in_stream_;
        Parser parser_;
        TransportCatalogue::Database& catalog_db_;
    };
}

namespace transport_catalogue::io {
        template <typename TOut>
        TOut Reader::Read() const {
            TOut result;
            in_stream_ >> result;
            in_stream_.get();
            return result;
        }
}