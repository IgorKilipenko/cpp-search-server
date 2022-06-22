#ifndef __BUS_MANAGER_H__
#define __BUS_MANAGER_H__

#include <cmath>
#include <iterator>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "datebase.hpp"

using namespace std;

enum class QueryType {
    NewBus,
    BusesForStop,
    StopsForBus,
    AllBuses,
};

struct Query {
    QueryType type;
    string bus;
    string stop;
    vector<string> stops;
};

struct BusesForStopResponse {
    // Наполните полями эту структуру
    string stop;
    vector<string> buses;
    bool no_stop;
};

struct StopsForBusResponse {
    // Наполните полями эту структуру
    string bus;
    Table route;
    bool no_bus;
    bool HasInterchange(const string& stop) const {
        if (!route.count(stop)) return false;
        return !route.at(stop).empty();
    }
};

struct AllBusesResponse {
    // Наполните полями эту структуру
    Table buses;
};

class BusManager {
   public:
    void AddBus(const string& bus, const vector<string>& stops);

    BusesForStopResponse GetBusesForStop(const string& stop) const;

    StopsForBusResponse GetStopsForBus(const string& bus) const;

    AllBusesResponse GetAllBuses() const;

   private:
    Datebase _db;
};

#endif  // __BUS_MANAGER_H__