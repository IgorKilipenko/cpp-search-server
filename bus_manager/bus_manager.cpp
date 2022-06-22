#include "bus_manager.hpp"

#include <cassert>
#include <iostream>
#include <istream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

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

ostream& operator<<(ostream& os, const vector<string>& vals) {
    string sep = ""s;
    for (const auto& v : vals) {
        os << sep << v;
        if (sep.empty()) sep = " ";
    }

    return os;
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
            os << " no interchange"s;
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
    for (const auto& [bus, stops] : r.buses) {
        os << "Bus " << bus << ": "s;
        os << stops;
        os << endl;
    }
    return os;
}

void BusManager::AddBus(const string& bus, const vector<string>& stops) {
    _db.AddBus(bus, stops);
    return;
}

BusesForStopResponse BusManager::GetBusesForStop(const string& stop) const {
    if (!_db.ContainStop(stop)) return {stop, {}, true};
    const auto& buses = _db.GetBuses(stop);
    return {stop, buses, false};
}

StopsForBusResponse BusManager::GetStopsForBus(const string& bus) const {
    if (!_db.ContainBus(bus)) return {bus, {}, true};
    const auto& route = _db.GetRoute(bus);
    return {bus, route, false};
}

AllBusesResponse BusManager::GetAllBuses() const {
    const auto& table = _db.GetBusesTable();
    const AllBusesResponse result{table.GetSortedTable()};
    return result;
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

    ss << 10 << endl;
    ss << "ALL_BUSES"s << endl;
    ss << "BUSES_FOR_STOP Marushkino"s << endl;
    ss << "STOPS_FOR_BUS 32K"s << endl;
    ss << "NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s << endl;
    ss << "NEW_BUS 32K 6 Tolstopaltsevo Marushkino Vnukovo Peredelkino Solntsevo Skolkovo"s << endl;
    ss << "BUSES_FOR_STOP Vnukovo"s << endl;
    ss << "NEW_BUS 950 6 Kokoshkino Marushkino Vnukovo Peredelkino Solntsevo Troparyovo"s << endl;
    ss << "NEW_BUS 272 4 Vnukovo Moskovsky Rumyantsevo Troparyovo"s << endl;
    ss << "STOPS_FOR_BUS 272"s << endl;
    ss << "ALL_BUSES"s << endl;

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