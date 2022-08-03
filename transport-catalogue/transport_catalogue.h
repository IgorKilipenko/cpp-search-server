#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue {
    using Coordinates = geo::Coordinates;

    struct Stop {
        std::string name;
        Coordinates coordinates;
        Stop() = default;
        Stop(std::string name, Coordinates&& coordinates): name{name}, coordinates{std::move(coordinates)} {}
        Stop(std::string name, const Coordinates& coordinates): name{name}, coordinates{coordinates} {}
    };

    class TransportCatalogue {
    public:
        template <typename LatLng>
        const Stop& AddStop(const std::string_view name, LatLng&& coordinates);

        const std::deque<Stop>& GetStops() const;

        const std::deque<Stop>& GetStop(const std::string_view name) const;

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Stop*> name_to_stop_;
    };
}

namespace transport_catalogue {
    template <typename LatLng>
    const Stop& TransportCatalogue::AddStop(const std::string_view name, LatLng&& coordinates) {
        return stops_.emplace_back(static_cast<std::string>(name), std::move(coordinates));
    }
}