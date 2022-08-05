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
    std::vector<std::string_view> SplitIntoWords(std::string_view str, const char ch = ' ', size_t max_count = 0);
}

namespace transport_catalogue::io {
    using namespace std::literals;

    class Parser {
        using Coordinates = geo::Coordinates;
        using StopsContainer = std::unordered_map<std::string_view, Coordinates>;

    public:
        // using StopRequest = std::pair<std::string_view, Coordinates>;
        using RouteRequest = std::tuple<std::string_view, std::vector<std::string_view>, bool>;

        struct DistanceBetween {
            double distance = 0.;
            std::string_view from_stop;
            std::string_view to_stop;

            DistanceBetween() = default;
            DistanceBetween(double distance, std::string_view from_stop, std::string_view to_stop)
                : distance(distance), from_stop(from_stop), to_stop(to_stop) {}
        };

        struct StopRequest {
            std::string_view name;
            Coordinates coordinates;
            std::vector<DistanceBetween> measured_distancies;

            StopRequest() = default;

            template <typename StringView, typename Coordinates, typename DistanceBetweenContainer>
            StopRequest(StringView&& name, Coordinates&& coordinates, DistanceBetweenContainer&& measured_distancies)
                : name{std::move(name)}, coordinates(std::move(coordinates)), measured_distancies(std::move(measured_distancies)) {}
        };

        struct RawRequest {
            enum class Type { ADD, GET, UNDEF };

            std::string_view command;
            std::string_view value;
            std::string_view args;
            Type type = Type::UNDEF;

            RawRequest() = default;
            RawRequest(std::string_view command, std::string_view value, std::string_view args, Type type)
                : command{command}, value{value}, args{args}, type{type} {}
        };
        struct Names {
            static constexpr const std::string_view STOP = "Stop"sv;
            static constexpr const std::string_view BUS = "Bus"sv;
        };

        StopRequest ParseStop(const RawRequest& req) const;

        RouteRequest ParseBusRoute(const RawRequest& req) const;

        std::shared_ptr<std::pair<std::string_view, std::string_view>> SplitKeyValue(const std::string_view str) const;

        RawRequest SplitRequest(const std::string_view str) const;

        Coordinates ParseLatLng(const std::string_view str, const char sep = ARGS_SEPARATOR) const;

        Coordinates ParseLatLng(const std::string_view lat_str, const std::string_view lng_str) const;

        bool IsRequestType(const std::string_view req, const std::string_view type) const;

        bool IsStopRequest(const std::string_view req) const;

        bool IsRouteRequest(const std::string_view req) const;

        bool IsCircularRoute(const std::string_view args) const;

        bool IsBidirectionalRoute(const std::string_view args) const;

        bool IsAddRequest(const std::string_view req) const;

        bool IsGetRequest(const std::string_view req) const;

        std::vector<DistanceBetween> ParseMeasuredDistancies(const std::string_view str, const std::string_view from_stop) const;

    private:
        static const char CIRCULAR_ROUTE_SEPARATOR = '>';
        static const char BIDIRECTIONAL_ROUTE_SEPARATOR = '-';
        static const char ARGS_SEPARATOR = ',';
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

        void ExecuteRequest(const Parser::RawRequest& raw_req, std::vector<Parser::DistanceBetween>& out_distances) const;

        void PorccessAddRequests(size_t n) const;

        void PorccessRequests() const;

        const Parser& GetParser() const {
            return parser_;
        }

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