#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <cassert>

#include "geo.h"

namespace transport_catalogue {

using Buses = std::set<std::string_view>;
struct Stop {
    std::string id;
    geo::Coordinates location;
    Buses buses;

    Stop(const std::string id, geo::Coordinates location)
        : id(std::move(std::string(id))), location(location) {}
};

using Route = std::vector<Stop*>;
struct Bus {
    std::string id;
    Route route;

    Bus(const std::string id, Route route)
        : id(std::move(std::string(id))), route(std::move(route)) {}
};

class TransportCatalogue {
public:
    // Методы построения:
    void AddBus(const std::string_view id, const std::vector<std::string_view>& stops);
    void AddStop(const std::string_view id, geo::Coordinates location);

    // Методы доступа:
    Bus* GetBus(const std::string_view id) const;
    Stop* GetStop(const std::string_view id) const;

    // Вспомогательные методы:
    size_t GetRouteUniqueStopsCount(const Route& route) const;
    double GetRouteLength(const Route& route) const;
    
private:
    // Массивы
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    // Индексы
    std::unordered_map<std::string_view, Bus*> buses_map_;
    std::unordered_map<std::string_view, Stop*> stops_map_;
};

} // end of namespace transport_catalogue