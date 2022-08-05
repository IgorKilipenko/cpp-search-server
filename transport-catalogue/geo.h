#pragma once

#include <cmath>
#include <iostream>
#include <type_traits>

namespace transport_catalogue::geo {
    static constexpr double PI() {
        return std::atan(1) * 4.;
    }
    static const double EARTH_RADIUS = 6371000.;

    struct Coordinates {
        double lat = 0.;
        double lng = 0.;

        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }

        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }

        Coordinates() : Coordinates(0., 0.) {}

        Coordinates(double lat, double lng) : lat{lat}, lng{lng} {}

        Coordinates(Coordinates&& other) : lat{std::move(other.lat)}, lng{std::move(other.lng)} {}

        Coordinates(const Coordinates& other) = default;

        Coordinates& operator=(const Coordinates& other) = default;

        Coordinates& operator=(Coordinates&& other) = default;
    };

    inline double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = PI() /*3.1415926535*/ / 180.;
        return std::acos(
                   std::sin(from.lat * dr) * std::sin(to.lat * dr) +
                   std::cos(from.lat * dr) * std::cos(to.lat * dr) * std::cos(std::abs(from.lng - to.lng) * dr)) *
               EARTH_RADIUS;
    }
}