#pragma once

#include <algorithm>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace transport_catalogue::data {
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
                std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, data::Coordinates>, bool> = true>
        Stop(String&& name, Coordinates&& coordinates) : name{std::move(name)}, coordinates{std::move(coordinates)} {}
    };

    using Route = std::vector<const Stop*>;

    struct Bus {
        std::string name;
        Route route;
        Bus() = default;
        template <
            typename String, typename Route,
            std::enable_if_t<std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Route>, data::Route>, bool> = true>
        Bus(String&& name, Route&& route) : name{std::move(name)}, route{std::move(route)} {}
    };

    template <class Owner>
    class Database {
        friend Owner;

    public:
        using StopsTable = std::deque<Stop>;
        using BusRoutesTable = std::deque<Bus>;
        using NameToStopView = std::unordered_map<std::string_view, const data::Stop*>;
        using NameToBusRoutesView = std::unordered_map<std::string_view, const data::Bus*>;

        const StopsTable& GetStopsTable() const {
            return stops_;
        }

        const BusRoutesTable& GetBusRoutesTable() const {
            return bus_routes_;
        }

        const NameToStopView& GetNameToStopView() const {
            return name_to_stop_;
        }

        template <typename Stop, std::enable_if_t<std::is_same_v<std::decay_t<Stop>, data::Stop>, bool> = true>
        const Stop& AddStop(Stop&& stop) {
            const Stop& new_stop = stops_.emplace_back(std::move(stop));
            name_to_stop_[new_stop.name] = &new_stop;
            return new_stop;
        }

        template <
            typename String, typename Coordinates,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_convertible_v<std::decay_t<Coordinates>, data::Coordinates>, bool> =
                true>
        const Stop& AddStop(String&& name, Coordinates&& coordinates) {
            return AddStop({std::move(name), std::move(coordinates)});
        }

        template <typename Bus, std::enable_if_t<std::is_same_v<std::decay_t<Bus>, data::Bus>, bool> = true>
        const Bus& AddBus(Bus&& bus_route) {
            const Bus& new_bus = bus_routes_.emplace_back(std::move(bus_route));
            name_to_bus_[new_bus.name] = &new_bus;
            return new_bus;
        }

        template <
            typename String, typename Route,
            std::enable_if_t<std::is_same_v<std::decay_t<String>, std::string> && std::is_convertible_v<std::decay_t<Route>, data::Route>, bool> =
                true>
        const Bus& AddBus(String&& name, Route&& route) {
            return AddBus(Bus{std::move(name), std::move(route)});
        }

        template <
            typename String, typename RawRouteContainer,
            std::enable_if_t<std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<RawRouteContainer>, std::vector<std::string_view>>, bool> =
                true>
        const Bus& AddBus(String&& name, RawRouteContainer&& stops) {
            Route route{stops.size()};
            std::transform(stops.begin(), stops.end(), route.begin(), [&](const std::string_view stop) {
                return name_to_stop_.at(stop);
            });
            return AddBus(std::move(name), std::move(route));
        }

        void LockDatabase() {
            mutex_.lock();
        }

        void UnlockDatabase() {
            mutex_.unlock();
        }

        std::lock_guard<std::mutex> LockGuard() {
            return std::lock_guard<std::mutex>{mutex_};
        }

    private:
        StopsTable stops_;
        BusRoutesTable bus_routes_;
        NameToStopView name_to_stop_;
        NameToBusRoutesView name_to_bus_;
        std::mutex mutex_;
    };
}

namespace transport_catalogue {
    using Coordinates = data::Coordinates;
    using Stop = data::Stop;

    class TransportCatalogue {
    public:
        using Database = data::Database<TransportCatalogue>;
        TransportCatalogue() : db_{new Database()} {}
        TransportCatalogue(std::shared_ptr<Database> db) : db_{db} {}

        template <
            typename String, typename Coordinates,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, transport_catalogue::Coordinates>,
                bool> = true>
        const Stop& AddStop(String&& name, Coordinates&& coordinates);

        const std::deque<Stop>& GetStops() const;

        const std::deque<Stop>& GetStop(const std::string_view name) const;

        const std::shared_ptr<Database> GetDatabase() const {
            return db_;
        }
        const Database& GetDatabaseReadOnly() const {
            return *db_;
        }

    private:
        std::shared_ptr<Database> db_;
    };

}

namespace transport_catalogue {
    template <
        typename String, typename Coordinates,
        std::enable_if_t<
            std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, transport_catalogue::Coordinates>, bool>>
    const Stop& TransportCatalogue::AddStop(String&& name, Coordinates&& coordinates) {
        return db_->AddStop(std::move(name), std::move(coordinates));
    }
}