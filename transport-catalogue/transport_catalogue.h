#pragma once

#include "domain.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <cassert>

namespace transport_catalogue {
    
    using namespace domain;

    class TransportCatalogue {
    public:
        // Методы построения:
        BusPtr AddBus(const std::string_view id, const std::vector<std::string_view>& stops, bool is_roundtrip);
        StopPtr AddStop(const std::string_view id, geo::Coordinates location = {});
        void AddDistance(const std::string_view from, const std::string_view to, double distance);

        // Методы доступа:
        BusPtr GetBus(const std::string_view id) const;
        StopPtr GetStop(const std::string_view id) const;
        double GetDistance(const StopPtr from_stop_ptr, const StopPtr to_stop_ptr) const;
        
        // Методы запросов:
        const StopBuses* GetBusesByStop(const StopPtr stop_ptr) const;
        const BusStat GetBusStat(const BusPtr bus_ptr) const;
        
        size_t GetRouteUniqueStopsCount(const BusPtr bus_ptr) const;
        double GetRouteLength(const BusPtr bus_ptr, bool use_geo_distance = false) const;

        // Возвращает отсортированный список маршрутов
        const Buses GetSortedBuses() const;

        // Возвращает отсортированный список остановок входящих в маршруты
        const Stops GetActiveSortedStops() const;

    private:
        // Массивы данных:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;

        // Индексы:
        std::unordered_map<std::string_view, BusPtr> buses_map_;
        std::unordered_map<std::string_view, StopPtr> stops_map_;

        std::unordered_map<std::string_view, StopBuses> stop_buses_map_;

        std::unordered_map<std::string_view,
            std::unordered_map<std::string_view, double>> distances_map_;
    };

} // end of namespace transport_catalogue