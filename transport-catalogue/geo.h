#pragma once

#include <cmath>

namespace transport_catalogue::geo {

struct Coordinates {
    double lat;
    double lng;

    // Конструктор преобразования
    Coordinates(double lat = std::nan(""), double lng = std::nan("")) 
        : lat(lat), lng(lng) {}

    // Оператор сравнения ==
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }

    // Оператор сравнения !=
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance(const Coordinates from, const Coordinates to) {
    // Обработка особого случая (расстояние до себя = 0.0)
    if (from == to) return 0.;

    using namespace std;
    static const double dr = 3.1415926535 / 180.;
    static const int kEarthRadius = 6371000;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
        + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * kEarthRadius;
}

} // end of namespace transport_catalogue::geo