#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <cassert>
#include <optional>

#include "geo.h"

namespace transport_catalogue {

using Route = std::vector<std::string>;
using Index = std::unordered_map<std::string, size_t>;
using StopBuses = std::set<std::string_view>;

struct Stop {
    std::string id;
    geo::Coordinates location;
    
    Stop(const std::string& stop_id, geo::Coordinates location)
        : id(stop_id), location(location) {}
};

struct Bus {
    std::string id;
    Route route;

    Bus(const std::string& bus_id, Route route)
        : id(bus_id), route(std::move(route)) {}
};

struct StopResult {
    bool found;
    const Bus& bus;
};

struct BusInfo {
    size_t stops_count;
    size_t unique_stops_count;
    double length;
};

struct StopInfo {
    StopBuses stop_buses;
};

class TransportCatalogue {
public:
    // Добавляет остановку в справочник
    void AddStop(const std::string_view stop_id, geo::Coordinates location);

    // Доавляет маршрут в справочник
    void AddBus(const std::string_view bus_id, const Route& route);
    
    // Возвращает индекс остановки по наименованию
    std::optional<size_t> GetStopIndex(const std::string_view bus_id) const;

    // Возвращает индекс маршрута по наименованию
    std::optional<size_t> GetBusIndex(const std::string_view bus_id) const;

    // Возвращает информацию о маршруте
    std::optional<BusInfo> GetBusInfo(const std::string_view bus_id) const;

    // Возвращает отсортированный список уникальных маршрутов, проходящих через остановку
    std::optional<StopInfo> GetStopInfo(const std::string_view stop_id) const;
    
private:
    // Массивы данных
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    // Индексы
    Index stops_index_map_;
    Index buses_index_map_;
    std::unordered_map<size_t, StopBuses> stop_buses_index_map_;

    // Возвращает индекс по наименованию
    std::optional<size_t> GetIndex(const std::string_view id, const Index& map) const;

    // Подсчитывает количество уникальных остановок маршрута
    size_t CountUniqueStops(const Route& route) const;

    // Вычисляет длину маршрута
    double CalculateRouteLength(const Route& route) const;

    // Обновляет индекс stop_buses_
    void UpdateStopBuses(const Route& route, const std::string_view bus_id);
};

} // end of namespace transport_catalogue