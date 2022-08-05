#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <string_view>
#include <vector>

#include "stat_reader.h"

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
    std::vector<std::string_view> SplitIntoWords(const std::string_view str, const char ch, size_t max_count) {
        if (str.empty()) {
            return {};
        }
        std::string_view str_cpy = str;
        std::vector<std::string_view> result;
        Trim(str_cpy);

        do {
            int64_t pos = str_cpy.find(ch, 0);
            std::string_view substr = (pos == static_cast<int64_t>(str_cpy.npos)) ? str_cpy.substr(0) : str_cpy.substr(0, pos);
            Trim(substr);
            result.push_back(std::move(substr));
            str_cpy.remove_prefix(std::min(str_cpy.find_first_not_of(ch, pos), str_cpy.size()));
            if (max_count && result.size() == max_count - 1) {
                Trim(str_cpy);
                result.push_back(std::move(str_cpy));
                break;
            }
        } while (!str_cpy.empty());

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

    void Reader::ExecuteRequest(const Parser::RawRequest& raw_req, std::vector<Parser::DistanceBetween>& out_distances) const {
        assert(raw_req.type == Parser::RawRequest::Type::ADD);
        assert(!raw_req.value.empty() && !raw_req.command.empty() && !raw_req.args.empty());

        if (parser_.IsStopRequest(raw_req.command)) {
            auto stop = parser_.ParseStop(raw_req);
            catalog_db_.AddStop(Stop{static_cast<std::string>(stop.name), std::move(stop.coordinates)});
            /*std::for_each(stop.measured_distancies.begin(), stop.measured_distancies.end(), [&out_distances](const auto& dist_btw) {
                //catalog_db_.AddMasuredDistance(dist_btw.from_stop, dist_btw.to_stop, dist_btw.distance);
                out_distances.push_back(dist_btw);
            });*/
            std::move(stop.measured_distancies.begin(), stop.measured_distancies.end(), std::back_inserter(out_distances));

        } else if (parser_.IsRouteRequest(raw_req.command)) {
            auto [name, route, _] = parser_.ParseBusRoute(raw_req);
            catalog_db_.AddBus(static_cast<std::string>(name), std::move(route));
        }
    }

    void Reader::PorccessAddRequests(size_t n) const {
        auto lines = ReadLines(n);
        std::vector<Parser::RawRequest> requests;
        requests.reserve(n);
        std::for_each(std::make_move_iterator(lines.begin()), std::make_move_iterator(lines.end()), [this, &requests](const std::string_view str) {
            if (parser_.IsAddRequest(str)) {
                requests.push_back(parser_.SplitRequest(str));
            }
        });
        std::sort(requests.begin(), requests.end(), [](const Parser::RawRequest& lhs, const Parser::RawRequest& rhs) {
            return (lhs.command == Parser::Names::STOP ? 0 : 1) < (rhs.command == Parser::Names::STOP ? 0 : 1);
        });

        std::vector<Parser::DistanceBetween> distances;
        distances.reserve(requests.size() * 4);
        std::for_each(
            std::make_move_iterator(requests.begin()), std::make_move_iterator(requests.end()),
            [this, &distances](const Parser::RawRequest& raw_req) {
                ExecuteRequest(raw_req, distances);
            });

        std::for_each(std::make_move_iterator(distances.begin()), std::make_move_iterator(distances.end()), [this](const auto& distance_btw) {
            catalog_db_.AddMasuredDistance(distance_btw.from_stop, distance_btw.to_stop, distance_btw.distance);
        });
    }

    void Reader::PorccessRequests() const {
        PorccessAddRequests(Read<size_t>());
    }
}

namespace transport_catalogue::io {
    Parser::StopRequest Parser::ParseStop(const RawRequest& req) const {
        assert(!req.value.empty() && !req.args.empty() && req.command == Names::STOP);

        auto args = detail::SplitIntoWords(req.args, ARGS_SEPARATOR, 3);
        assert(args.size() >= 2);

        std::vector<DistanceBetween> distancies;
        if (args.size() > 2) {
            distancies = ParseMeasuredDistancies(args.back(), req.value);
        }

        return {req.value, ParseLatLng(args[0], args[1]), std::move(distancies)};
    }

    Parser::RouteRequest Parser::ParseBusRoute(const RawRequest& req) const {
        assert(!req.value.empty() && !req.args.empty() && req.command == Names::BUS);
        assert(IsCircularRoute(req.args) || IsBidirectionalRoute(req.args));

        bool is_circular = IsCircularRoute(req.args);

        std::vector<std::string_view> stops =
            detail::SplitIntoWords(req.args, is_circular ? CIRCULAR_ROUTE_SEPARATOR : BIDIRECTIONAL_ROUTE_SEPARATOR);
        if (!is_circular && stops.size() > 1) {
            size_t old_size = stops.size();
            stops.resize(old_size * 2 - 1);
            std::copy(stops.begin(), stops.begin() + old_size - 1, stops.rbegin());
        }

        RouteRequest result{req.value, std::move(stops), is_circular};

        assert(!IsCircularRoute(req.args) || std::get<1>(result).front() == std::get<1>(result).back());

        return result;
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
        using namespace std::string_view_literals;

        assert(IsAddRequest(str) || IsGetRequest(str));

        static constexpr const std::string_view empty = ""sv;

        Parser::RawRequest::Type req_type = IsAddRequest(str) ? Parser::RawRequest::Type::ADD : Parser::RawRequest::Type::GET;
        std::vector<std::string_view> data;
        std::string_view args = empty;

        if (req_type == Parser::RawRequest::Type::ADD) {
            auto key_val_ptr = SplitKeyValue(str);
            req_type = key_val_ptr == nullptr ? Parser::RawRequest::Type::GET : Parser::RawRequest::Type::ADD;
            data = detail::SplitIntoWords(key_val_ptr->first, ' ', 2);
            args = key_val_ptr->second;
        } else {
            data = detail::SplitIntoWords(str, ' ', 2);
        }
        assert(data.size() == 2);
        return RawRequest(data[0], data[1], args, req_type);
    }

    Coordinates Parser::ParseLatLng(const std::string_view str, const char sep) const {
        auto point = detail::SplitIntoWords(str, sep);
        assert(point.size() >= 2);
        return ParseLatLng(point[0], point[1]);
    }

    Coordinates Parser::ParseLatLng(const std::string_view lat_str, const std::string_view lng_str) const {
        return {std::stod(static_cast<std::string>(lat_str)), std::stod(static_cast<std::string>(lng_str))};
    }

    std::vector<Parser::DistanceBetween> Parser::ParseMeasuredDistancies(const std::string_view str, const std::string_view from_stop) const {
        auto distancies = detail::SplitIntoWords(str, ARGS_SEPARATOR);
        if (distancies.empty()) {
            return {};
        }

        std::vector<Parser::DistanceBetween> dists_btw;
        dists_btw.reserve(distancies.size());

        std::for_each(
            std::make_move_iterator(distancies.begin()), std::make_move_iterator(distancies.end()),
            [&dists_btw, from_stop](const std::string_view str) {
                static const std::string_view sep = " to "sv;
                size_t idx = str.find(sep);
                assert(idx != str.npos);

                std::string_view to_stop_name = str.substr(idx + sep.length());
                detail::Trim(to_stop_name);

                std::string_view distance_str = str.substr(0, idx);
                detail::Trim(distance_str);
                assert(distance_str.find('.') != str.npos || distance_str.find('m') != str.npos);
                detail::TrimEnd(distance_str, 'm');

                dists_btw.push_back({std::stod(static_cast<std::string>(distance_str)), from_stop, to_stop_name});
            });

        return dists_btw;
    }

    bool Parser::IsRequestType(const std::string_view req, const std::string_view type) const {
        return type == req.substr(0, type.size());
    }

    bool Parser::IsStopRequest(const std::string_view req) const {
        return IsRequestType(req, Names::STOP);
    }

    bool Parser::IsRouteRequest(const std::string_view req) const {
        return IsRequestType(req, Names::BUS);
    }

    bool Parser::IsCircularRoute(const std::string_view args) const {
        return args.find(CIRCULAR_ROUTE_SEPARATOR) != args.npos;
    }

    bool Parser::IsBidirectionalRoute(const std::string_view args) const {
        return args.find(BIDIRECTIONAL_ROUTE_SEPARATOR) != args.npos;
    }

    bool Parser::IsAddRequest(const std::string_view req) const {
        bool result = IsRequestType(req, Names::BUS) || IsRequestType(req, Names::STOP);
        return result && req.find(':') != req.npos;
    }

    bool Parser::IsGetRequest(const std::string_view req) const {
        bool result = IsRequestType(req, Names::BUS) || IsRequestType(req, Names::STOP);
        return result && req.find(':') == req.npos;
    }
}