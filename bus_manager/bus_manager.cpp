#include "bus_manager.hpp"

#include <cassert>
#include <iostream>
#include <istream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "datebase.hpp"

using namespace std;

QueryType parseQueryType(const string& str) {
    const auto _to_lower = [](const string& str) {
        return accumulate(str.begin(), str.end(), ""s, [](string curr, const char c) {
            if (c != '_' && c != ' ') curr.push_back(tolower(c));
            return curr;
        });
    };
    string query = _to_lower(str);
    if (query == _to_lower("NewBus"s)) {
        return QueryType::NewBus;
    } else if (query == _to_lower("BusesForStop"s)) {
        return QueryType::BusesForStop;
    } else if (query == _to_lower("AllBuses"s)) {
        return QueryType::AllBuses;
    } else if (query == _to_lower("StopsForBus"s)) {
        return QueryType::StopsForBus;
    }
    throw "Invalid query string";
}

ostream& print_vector(ostream& os, const vector<string>& vals) {
    string sep = ""s;
    for (const auto& v : vals) {
        os << sep << v;
        if (sep.empty()) sep = " ";
    }

    return os;
}

ostream& operator<<(ostream& os, const vector<string>& vals) {
    return print_vector(os, vals);
}

istream& operator>>(istream& is, Query& q) {
    // Реализуйте эту функцию
    string type_str;
    is >> type_str;
    q.type = parseQueryType(type_str);

    if (q.type == QueryType::AllBuses) {
    } else if (q.type == QueryType::BusesForStop) {
        is >> q.stop;
    } else if (q.type == QueryType::StopsForBus) {
        is >> q.bus;
    } else if (q.type == QueryType::NewBus) {
        is >> q.bus;
        int stop_count;
        is >> stop_count;
        if (!stop_count) return is;

        q.stops = vector<string>(stop_count, ""s);
        for (int i = 0; i < stop_count; i++) is >> q.stops[i];
    }

    return is;
}

ostream& operator<<(ostream& os, const BusesForStopResponse& r) {
    // Реализуйте эту функцию
    if (r.no_stop) {
        cout << "No stop"s;
        return os;
    }

    string sep = ""s;
    for (const auto& bus : r.buses) {
        os << sep << bus;
        if (sep.empty()) sep = " ";
    }

    return os;
}

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
    // Реализуйте эту функцию
    if (r.no_bus) {
        os << "No bus"s;
        return os;
    }

    bool is_first = true;
    for (const auto& [stop, buses] : r.route) {
        if (!is_first) os << endl;
        os << "Stop " << stop << ": "s;
        if (!r.HasInterchange(stop)) {
            os << "no interchange"s;
            if (is_first) is_first = false;
            continue;
        }
        os << buses;
        if (is_first) is_first = false;
    }

    return os;
}

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
    // Реализуйте эту функцию
    if (!r.buses.size()) {
        os << "No buses"s;
        return os;
    }
    bool is_first = true;
    for (const auto& [bus, stops] : r.buses) {
        if (!is_first) os << endl;
        os << "Bus " << bus << ": "s;
        print_vector(os, stops);
        if (is_first) is_first = false;
    }
    return os;
}

void BusManager::AddBus(const string& bus, const vector<string>& stops) {
    _buses_to_stops[bus] = stops;
    for (const string& stop : stops) {
        _stops_to_buses[stop].push_back(bus);
    }
}

BusesForStopResponse BusManager::GetBusesForStop(const string& stop) const {
    if (_stops_to_buses.count(stop) == 0) {
        return {stop, {}, true};
    }
    return {stop, _stops_to_buses.at(stop), false};
}

StopsForBusResponse BusManager::GetStopsForBus(const string& bus) const {
    if (_buses_to_stops.count(bus) == 0) {
        return {bus, {}, true};
    }
    Table route{};
    for (const string& stop : _buses_to_stops.at(bus)) {
        pair<string, vector<string>> route_item(stop, {});
        for (const string& other_bus : _stops_to_buses.at(stop)) {
            if (bus != other_bus) {
                route_item.second.push_back(other_bus);
            }
        }
        route.push_back(route_item);
    }
    return {bus, route, false};
}

AllBusesResponse BusManager::GetAllBuses() const {
    if (_buses_to_stops.empty()) {
        return {};
    }
    map<string, vector<string>> result{};
    for (const auto& [bus, stops] : _buses_to_stops) {
        result[bus] = stops;
    }
    return {result};
}

// Не меняя тела функции main, реализуйте функции и классы выше

int main() {
    int query_count;
    Query q;

    /*
        cin >> query_count;

        BusManager bm;
        for (int i = 0; i < query_count; ++i) {
            cin >> q;
            switch (q.type) {
                case QueryType::NewBus:
                    bm.AddBus(q.bus, q.stops);
                    break;
                case QueryType::BusesForStop:
                    cout << bm.GetBusesForStop(q.stop) << endl;
                    break;
                case QueryType::StopsForBus:
                    cout << bm.GetStopsForBus(q.bus) << endl;
                    break;
                case QueryType::AllBuses:
                    cout << bm.GetAllBuses() << endl;
                    break;
            }
        }
    */

    stringstream ss;

    ss << 2 << endl;
    ss << "NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s << endl;
    ss << "ALL_BUSES"s << endl;
    ss << "NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s << endl;
    /*ss << "ALL_BUSES"s << endl;
    ss << "BUSES_FOR_STOP Marushkino"s << endl;
    ss << "STOPS_FOR_BUS 32K"s << endl;
    ss << "NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s << endl;
    ss << "NEW_BUS 32K 6 Tolstopaltsevo Marushkino Vnukovo Peredelkino Solntsevo Skolkovo"s << endl;
    ss << "BUSES_FOR_STOP Vnukovo"s << endl;
    ss << "NEW_BUS 950 6 Kokoshkino Marushkino Vnukovo Peredelkino Solntsevo Troparyovo"s << endl;
    ss << "NEW_BUS 272 4 Vnukovo Moskovsky Rumyantsevo Troparyovo"s << endl;
    ss << "STOPS_FOR_BUS 272"s << endl;
    ss << "STOPS_FOR_BUS 32"s << endl;
    ss << "STOPS_FOR_BUS 950"s << endl;
    ss << "ALL_BUSES"s << endl;*/

    ss >> query_count;

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        ss >> q;
        switch (q.type) {
            case QueryType::NewBus:
                bm.AddBus(q.bus, q.stops);
                break;
            case QueryType::BusesForStop:
                cout << bm.GetBusesForStop(q.stop) << endl;
                break;
            case QueryType::StopsForBus:
                cout << bm.GetStopsForBus(q.bus) << endl;
                break;
            case QueryType::AllBuses:
                cout << bm.GetAllBuses() << endl;
                break;
        }
    }
}

/*
No buses
No stop
No bus
32 32K
Stop Vnukovo: 32 32K 950
Stop Moskovsky: no interchange
Stop Rumyantsevo: no interchange
Stop Troparyovo: 950
Bus 272: Vnukovo Moskovsky Rumyantsevo Troparyovo
Bus 32: Tolstopaltsevo Marushkino Vnukovo
Bus 32K: Tolstopaltsevo Marushkino Vnukovo Peredelkino Solntsevo Skolkovo
Bus 950: Kokoshkino Marushkino Vnukovo Peredelkino Solntsevo Troparyovo

***************************************************************************
No buses
No stop
No bus
32 32K
Stop Vnukovo: 32 32K 950
Stop Moskovsky: no interchange
Stop Rumyantsevo: no interchange
Stop Troparyovo: 950
Bus 272: Vnukovo Moskovsky Rumyantsevo Troparyovo
Bus 32: Tolstopaltsevo Marushkino Vnukovo
Bus 32K: Tolstopaltsevo Marushkino Vnukovo Peredelkino Solntsevo Skolkovo
Bus 950: Kokoshkino Marushkino Vnukovo Peredelkino Solntsevo Troparyovo


No buses
No stop
No bus
32 32K
Stop Vnukovo: 32 32K 950
Stop Moskovsky: no interchange
Stop Rumyantsevo: no interchange
Stop Troparyovo: 950
Bus 272: Vnukovo Moskovsky Rumyantsevo Troparyovo
Bus 32: Tolstopaltsevo Marushkino Vnukovo
Bus 32K: Tolstopaltsevo Marushkino Vnukovo Peredelkino Solntsevo Skolkovo
Bus 950: Kokoshkino Marushkino Vnukovo Peredelkino Solntsevo Troparyovo

No buses
No stop
No bus
32 32K
Stop Vnukovo: 32 32K 950
Stop Moskovsky: no interchange
Stop Rumyantsevo: no interchange
Stop Troparyovo: 950
Bus 272: Vnukovo Moskovsky Rumyantsevo Troparyovo
Bus 32: Tolstopaltsevo Marushkino Vnukovo
Bus 32K: Tolstopaltsevo Marushkino Vnukovo Peredelkino Solntsevo Skolkovo
Bus 950: Kokoshkino Marushkino Vnukovo Peredelkino Solntsevo Troparyovo
*/