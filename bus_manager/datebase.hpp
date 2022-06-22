#ifndef __DATEBASE_H__
#define __DATEBASE_H__

#include <memory>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

class Table : public vector<pair<string, vector<string>>> {
   public:
    int UpdateKeysValues(const vector<string>& keys, const string& value) {
        int update_count = 0;
        for (const auto& key : keys) {
            auto& curr_values = (*this)[key];
            if (!std::count(curr_values.begin(), curr_values.end(), value)) {
                curr_values.push_back(value);
                ++update_count;
            }
        }

        return update_count;
    }

    /**
     * @brief Synchronization of dependent tables
     *
     */
    void SyncTables() {
        throw "Not implemented yet";
    }

    int count(const string& key) const {
        return count_if(this->begin(), this->end(), [&key](const auto& item) {
            return item.first == key;
        });
    }

    vector<string>& operator[](const string& key) {
        if (!this->count(key)) {
            this->push_back({key, {}});
            return this->back().second;
        }

        for (auto& item : *this) {
            if (item.first == key) return item.second;
        }

        throw "Error: Unknown key";
    }

    const vector<string>& at(const string& key) const {
        for (auto& item : *this) {
            if (item.first == key) return item.second;
        }
        throw "Argument not found: "s + key;
    }

    Table GetSortedTable() const {
        Table result = *this;
        sort(result.begin(), result.end(), [](const auto& lv, const auto& rv) {
            return lv.first < rv.first;
        });
        return result;
    }

   private:
    set<shared_ptr<Table>> _ref_tables;
};
class Datebase {
   public:
    const vector<string>& AddBus(const string& bus, const vector<string>& stops) {
        const auto& curr_buses = addToTable(bus, stops, _buses_to_stops);
        _stops_to_buses.UpdateKeysValues(stops, bus);
        return curr_buses;
    }

    const vector<string>& AddStop(const string& stop, const vector<string>& buses) {
        const auto& curr_stops = addToTable(stop, buses, _stops_to_buses);
        _buses_to_stops.UpdateKeysValues(buses, stop);
        return curr_stops;
    }

    const Table& GetBusesTable() const {
        return _buses_to_stops;
    }
    const Table& GetStopsTable() const {
        return _stops_to_buses;
    }

    int ContainBus(const string& bus) const {
        // return _buses_to_stops.count(bus);
        return count_if(_buses_to_stops.begin(), _buses_to_stops.end(), [&bus](const auto& item) {
            return item.first == bus;
        });
    }

    int ContainStop(const string& stop) const {
        // return _buses_to_stops.count(stop);
        return count_if(_stops_to_buses.begin(), _stops_to_buses.end(), [&stop](const auto& item) {
            return item.first == stop;
        });
    }

    int GetBusesCount() const {
        return _stops_to_buses.size();
    }

    vector<string> GetBuses() const {
        vector<string> result{};
        result.reserve(_buses_to_stops.size());
        for (const auto& [bus, _] : _buses_to_stops) {
            result.push_back(bus);
        }
        return result;
    }

    const vector<string>& GetBuses(const string& stop) const {
        return _stops_to_buses.at(stop);
    }

    vector<string> GetStops() const {
        vector<string> result{};
        result.reserve(_stops_to_buses.size());
        for (const auto& [stop, _] : _stops_to_buses) {
            result.push_back(stop);
        }
        return result;
    }

    const vector<string>& GetStops(const string& bus) const {
        return _buses_to_stops.at(bus);
    }

    const Table GetRoute(const string& bus) const {
        const auto erase_current = [&bus](vector<std::string>& buses) {
            if (buses.empty()) return;

            int curr_bus_idx = -1;
            string curr_bus = "";
            for (int i = 0; i < buses.size(); i++) {
                if (buses[i] == bus) {
                    curr_bus_idx = i;
                    curr_bus = buses[i];
                    break;
                }
            }

            if (curr_bus_idx < 0) {
                throw "Invalid bus number: " + curr_bus;
            }

            if (curr_bus_idx >= 0) {
                buses.erase(buses.begin() + curr_bus_idx);
            }
        };
        const auto& stops = _buses_to_stops.at(bus);
        Table result{};
        for (const auto& stop : stops) {
            auto buses = GetBuses(stop);
            if (buses.empty()) continue;
            erase_current(buses);
            result.push_back({stop, buses});
        }

        return result;
    }

   private:
    Table _buses_to_stops;
    Table _stops_to_buses;

    static vector<string>& addToTable(const string& key, const vector<string>& values, Table& table) {
        auto& curr_values = table[key];
        curr_values.insert(curr_values.end(), values.begin(), values.end());
        return curr_values;
    }
};

#endif  // __DATEBASE_H__