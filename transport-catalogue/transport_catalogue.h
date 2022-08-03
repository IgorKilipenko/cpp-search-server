#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue {
    using Coordinates = geo::Coordinates;

    struct Stop {
        std::string name;
        Coordinates coordinates;
        Stop() = default;
        /*Stop(std::string&& name, Coordinates&& coordinates): name{std::move(name)}, coordinates{std::move(coordinates)} {}
        Stop(const std::string& name, const Coordinates& coordinates): name{name}, coordinates{coordinates} {}*/
        template <
            typename String, typename Coordinates,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, transport_catalogue::Coordinates>,
                bool> = true>
        Stop(String&& name, Coordinates&& coordinates) : name{std::move(name)}, coordinates{std::move(coordinates)} {}
    };

    class TransportCatalogue {
    public:
        template <
            typename String, typename Coordinates,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, transport_catalogue::Coordinates>,
                bool> = true>
        const Stop& AddStop(String&& name, Coordinates&& coordinates);

        const std::deque<Stop>& GetStops() const;

        const std::deque<Stop>& GetStop(const std::string_view name) const;

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Stop*> name_to_stop_;
    };
}

namespace transport_catalogue {
    template <
        typename String, typename Coordinates,
        std::enable_if_t<
            std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, transport_catalogue::Coordinates>, bool>>
    const Stop& TransportCatalogue::AddStop(String&& name, Coordinates&& coordinates) {
        return stops_.emplace_back(std::move(name), std::move(coordinates));
    }
}