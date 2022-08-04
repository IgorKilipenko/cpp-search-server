#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue::database {
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
                std::is_same_v<std::decay_t<String>, std::string> && std::is_same_v<std::decay_t<Coordinates>, database::Coordinates>, bool> = true>
        Stop(String&& name, Coordinates&& coordinates) : name{std::move(name)}, coordinates{std::move(coordinates)} {}
    };

    template <class Owner>
    class Database {
        friend Owner;

    public:
        using StopsTable = std::deque<Stop>;
        using NameToStopView = std::unordered_map<std::string_view, const database::Stop*>;

        const StopsTable& GetStopsTable() const {
            return stops_;
        }

        const NameToStopView& GetNameToStopView() const {
            return name_to_stop_;
        }

        template <typename Stop, std::enable_if_t<std::is_same_v<std::decay_t<Stop>, database::Stop>, bool> = true>
        const Stop& AddStop(Stop&& stop) {
            return stops_.emplace_back(std::move(stop));
        }

        template <
            typename String, typename Coordinates,
            std::enable_if_t<
                std::is_same_v<std::decay_t<String>, std::string> && std::is_convertible_v<std::decay_t<Coordinates>, database::Coordinates>, bool> = true>
        const Stop& AddStop(String&& name, Coordinates&& coordinates) {
            return stops_.emplace(std::move(name), std::move(coordinates));
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
        NameToStopView name_to_stop_;
        std::mutex mutex_;
    };
}

namespace transport_catalogue {
    using Coordinates = database::Coordinates;
    using Stop = database::Stop;

    class TransportCatalogue {
    public:
        using Database = database::Database<TransportCatalogue>;
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
        // return db_.stops_.emplace(std::move(name), std::move(coordinates));
        //return db_->AddStop({std::move(name), std::move(coordinates)});
        return db_->AddStop(std::move(name), std::move(coordinates));
    }
}