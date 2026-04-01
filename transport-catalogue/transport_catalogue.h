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

struct Stop;
using Route = std::vector<Stop*>;
using StopBuses = std::set<std::string_view>;

struct Stop {
    std::string id;
    geo::Coordinates location;

    Stop(const std::string id, geo::Coordinates location)
        : id(std::move(id)), location(location) {}
};

struct Bus {
    std::string id;
    Route route;

    Bus(const std::string id, Route route)
        : id(std::move(id)), route(std::move(route)) {}
};

struct BusInfo {
    size_t total_stops;
    size_t unique_stops;
    double length_km;
};

class TransportCatalogue {
public:
    // Методы построения:
    void AddBus(const std::string_view id, const std::vector<std::string_view>& stops);
    void AddStop(const std::string_view id, geo::Coordinates location);

    // Методы доступа:
    const Bus* GetBus(const std::string_view id) const;
    const Stop* GetStop(const std::string_view id) const;
    
    // Методы запросов:
    const StopBuses* GetStopBuses(const Stop* stop_ptr) const;
    const BusInfo GetBusInfo(const Bus* bus_ptr) const;

    size_t GetUniqueStopsCount(const Bus* bus_ptr) const;
    double GetLength(const Bus* bus_ptr) const;

private:
    // Массивы
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    // Индексы
    std::unordered_map<std::string_view, Bus*> buses_map_;
    std::unordered_map<std::string_view, Stop*> stops_map_;

    std::unordered_map<std::string_view, StopBuses> stop_buses_map_;
};

} // end of namespace transport_catalogue