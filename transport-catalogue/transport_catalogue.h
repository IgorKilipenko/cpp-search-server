#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <initializer_list>
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
        template <
            typename String, typename Coordinates,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, data::Coordinates>, bool> = true>
        Stop(String&& name, Coordinates&& coordinates) : name{std::move(name)}, coordinates{std::move(coordinates)} {}
        Stop(Stop&& other) : name{std::move(other.name)}, coordinates{std::move(other.coordinates)} {
            std::cerr << "Stop move constructor" << std::endl;
        }
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
        Bus(Bus&& other) : name{std::move(other.name)}, route{std::move(other.route)} {
            std::cerr << "Bus move constructor" << std::endl;
        }
        /*Bus(const Bus& other) : name{other.name}, route{other.route} {
            std::cerr << "Bus copy constructor" << std::endl;
        }
        Bus& operator=(const Bus& other) = default;*/
    };

    struct BusRouteInfo {
        size_t total_stops{};
        size_t unique_stops{};
        double route_length{};
        double route_distance{};
    };

    class Hasher {
    public:
        size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
            return pointer_hasher_(stops.first) + pointer_hasher_(stops.second) * INDEX;
        }
        template <typename T>
        size_t operator()(std::initializer_list<const T*> items) const {
            size_t hash = 0;
            for (const T* cur : items) {
                hash = hash * INDEX + pointer_hasher_(cur);
            }
            return hash;
        }

    private:
        std::hash<const void*> pointer_hasher_;
        static const size_t INDEX = 42;
    };

    template <class Owner>
    class Database {
        friend Owner;

    public:
        using StopsTable = std::deque<Stop>;
        using BusRoutesTable = std::deque<Bus>;
        using NameToStopView = std::unordered_map<std::string_view, const data::Stop*>;
        using NameToBusRoutesView = std::unordered_map<std::string_view, const data::Bus*>;

        const StopsTable& GetStopsTable() const;

        const BusRoutesTable& GetBusRoutesTable() const;

        const NameToStopView& GetNameToStopView() const;

        template <typename Stop, std::enable_if_t<std::is_same_v<std::decay_t<Stop>, data::Stop>, bool> = true>
        const Stop& AddStop(Stop&& stop);

        template <
            typename String, typename Coordinates,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_convertible_v<std::decay_t<Coordinates>, data::Coordinates>, bool> =
                true>
        const Stop& AddStop(String&& name, Coordinates&& coordinates);

        template <typename Bus, std::enable_if_t<std::is_same_v<std::decay_t<Bus>, data::Bus>, bool> = true>
        const Bus& AddBus(Bus&& bus_route);

        template <
            typename String, typename Route,
            std::enable_if_t<std::is_same_v<std::decay_t<String>, std::string> && std::is_convertible_v<std::decay_t<Route>, data::Route>, bool> =
                true>
        const Bus& AddBus(String&& name, Route&& route);

        template <
            typename String, typename RawRouteContainer,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<RawRouteContainer>, std::vector<std::string_view>>,
                bool> = true>
        const Bus& AddBus(String&& name, RawRouteContainer&& stops);

        template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool> = true>
        const Bus* const GetBus(StringView&& name) const;

        template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool> = true>
        const Stop* const GetStop(StringView&& name) const;

    private:
        StopsTable stops_;
        BusRoutesTable bus_routes_;
        NameToStopView name_to_stop_;
        NameToBusRoutesView name_to_bus_;
        std::mutex mutex_;

        template <
            typename StringView, typename TableView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool> = true>
        const Stop* const GetItem(StringView&& name, const TableView& table) const;

        void LockDatabase() {
            mutex_.lock();
        }

        void UnlockDatabase() {
            mutex_.unlock();
        }

        std::lock_guard<std::mutex> LockGuard() {
            return std::lock_guard<std::mutex>{mutex_};
        }
    };
}

namespace transport_catalogue {
    using Coordinates = data::Coordinates;
    using Stop = data::Stop;
    using Bus = data::Bus;

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

        template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool> = true>
        const Bus* const GetBus(StringView&& name) const;

        template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool> = true>
        const std::deque<Stop>& GetStop(StringView&& name) const;

        const std::deque<Stop>& GetStops() const;

        const std::shared_ptr<Database> GetDatabaseForWrite() const;

        const std::shared_ptr<const Database> GetDatabaseReadOnly() const;

    private:
        std::shared_ptr<Database> db_;
    };

}

namespace transport_catalogue::data {

    template <class Owner>
    template <typename Stop, std::enable_if_t<std::is_same_v<std::decay_t<Stop>, data::Stop>, bool>>
    const Stop& Database<Owner>::AddStop(Stop&& stop) {
        const Stop& new_stop = stops_.emplace_back(std::move(stop));
        name_to_stop_[new_stop.name] = &new_stop;
        return new_stop;
    }

    template <class Owner>
    template <
        typename String, typename Coordinates,
        std::enable_if_t<
            std::is_same_v<std::decay_t<String>, std::string> && std::is_convertible_v<std::decay_t<Coordinates>, data::Coordinates>, bool>>
    const Stop& Database<Owner>::AddStop(String&& name, Coordinates&& coordinates) {
        return AddStop({std::move(name), std::move(coordinates)});
    }

    template <class Owner>
    template <typename Bus, std::enable_if_t<std::is_same_v<std::decay_t<Bus>, data::Bus>, bool>>
    const Bus& Database<Owner>::AddBus(Bus&& bus_route) {
        const Bus& new_bus = bus_routes_.emplace_back(std::move(bus_route));
        name_to_bus_[new_bus.name] = &new_bus;
        return new_bus;
    }

    template <class Owner>
    template <
        typename String, typename Route,
        std::enable_if_t<std::is_same_v<std::decay_t<String>, std::string> && std::is_convertible_v<std::decay_t<Route>, data::Route>, bool>>
    const Bus& Database<Owner>::AddBus(String&& name, Route&& route) {
        return AddBus(Bus{std::move(name), std::move(route)});
    }

    template <class Owner>
    template <
        typename String, typename RawRouteContainer,
        std::enable_if_t<
            std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<RawRouteContainer>, std::vector<std::string_view>>,
            bool>>
    const Bus& Database<Owner>::AddBus(String&& name, RawRouteContainer&& stops) {
        Route route{stops.size()};
        std::transform(stops.begin(), stops.end(), route.begin(), [&](const std::string_view stop) {
            return name_to_stop_.at(stop);
        });
        return AddBus(std::move(name), std::move(route));
    }

    template <class Owner>
    template <typename StringView, typename TableView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool>>
    const Stop* const Database<Owner>::GetItem(StringView&& name, const TableView& table) const {
        auto ptr = table.find(std::move(name));
        if (ptr == table.end()) {
            return nullptr;
        }
        return ptr->second;
    }

    template <class Owner>
    template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool>>
    const Bus* const Database<Owner>::GetBus(StringView&& name) const {
        /*auto ptr = name_to_bus_.find(std::move(name));
        if (ptr == name_to_bus_.end()) {
            return nullptr;
        }
        return ptr->second;*/

        return GetItem(std::move(name), name_to_bus_);
    }

    template <class Owner>
    template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool>>
    const Stop* const Database<Owner>::GetStop(StringView&& name) const {
        /*auto ptr = name_to_stop_.find(std::move(name));
        if (ptr == name_to_stop_.end()) {
            return nullptr;
        }
        return ptr->second;*/

        return GetItem(std::move(name), name_to_stop_);
    }

    template <class Owner>
    const typename Database<Owner>::StopsTable& Database<Owner>::GetStopsTable() const {
        return stops_;
    }

    template <class Owner>
    const typename Database<Owner>::BusRoutesTable& Database<Owner>::GetBusRoutesTable() const {
        return bus_routes_;
    }

    template <class Owner>
    const typename Database<Owner>::NameToStopView& Database<Owner>::GetNameToStopView() const {
        return name_to_stop_;
    }
}

namespace transport_catalogue {
    template <
        typename String, typename Coordinates,
        std::enable_if_t<
            std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, transport_catalogue::Coordinates>, bool>>
    const Stop& TransportCatalogue::AddStop(String&& name, Coordinates&& coordinates) {
        return db_->AddStop(std::move(name), std::move(coordinates));
    }

    template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool>>
    const Bus* const TransportCatalogue::GetBus(StringView&& name) const {
        return db_->GetBus(std::move(name));
    }

    template <typename StringView, std::enable_if_t<std::is_convertible_v<std::decay_t<StringView>, std::string_view>, bool>>
    const std::deque<Stop>& TransportCatalogue::GetStop(StringView&& name) const {
        return db_->GetStop(std::move(name));
    }
}
