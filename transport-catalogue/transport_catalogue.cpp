#include "transport_catalogue.h"


namespace transport_catalogue {

// Возвращает хэш строки
size_t TransportCatalogue::Hash(const std::string& str) const {
    return std::hash<std::string>{}(str);
}

// Подсчитывает количество уникальных остановок маршрута
size_t TransportCatalogue::CountUniqueStops(const Route& route) const {
    std::unordered_set<std::string> unique_stops;
    for (const auto& stop : route) {
        unique_stops.insert(stop);
    }
    return unique_stops.size();
}

// Вычисляет длину маршрута
double TransportCatalogue::CalculateRouteLength(const Route& route) const {
    if (route.empty()) return 0.0;

    double length = 0.0;
    const Stop* prev_stop = nullptr;
    for (const auto& id : route) {
        auto stop_it = stops_.find(Hash(id));
        assert(stop_it != stops_.end());

        const Stop& current_stop = stop_it->second;
        if (prev_stop) {
            length += geo::ComputeDistance(prev_stop->location, current_stop.location);
        }
        prev_stop = &current_stop;
    }

    return length;
}

// Обновляет индекс stop_buses_
void TransportCatalogue::UpdateStopBuses(const Bus& bus) {
    for (const auto& stop_id : bus.route) {
        Buses& buses = stop_buses_[Hash(stop_id)];
        buses.insert(bus.id);
    }
}

// Добавляет остановку в справочник
void TransportCatalogue::AddStop(const std::string& id, geo::Coordinates location) {
    stops_.try_emplace(Hash(id), id, location);
}

// Доавляет маршрут в справочник
void TransportCatalogue::AddBus(const std::string& id, Route& route) {
    auto [it, inserted] = buses_.try_emplace(Hash(id), id, std::move(route));
    if (inserted) UpdateStopBuses(it->second);
}

// Возвращает информацию о маршруте
bool TransportCatalogue::GetBusInfo(const std::string& id, BusInfo* info) const {
    auto bus_it = buses_.find(Hash(id));
    if (bus_it == buses_.end()) return false;

    const Bus& bus = bus_it->second;
    info->stops_count = bus.route.size();
    info->unique_stops_count = CountUniqueStops(bus.route);
    info->length = CalculateRouteLength(bus.route);
    return true;
}

// Возвращает отсортированный список уникальных маршрутов, проходящих через остановку
bool TransportCatalogue::GetStopInfo(const std::string& id, StopInfo* info) const {
    auto hash = Hash(id);

    auto stop_it = stops_.find(hash);
    if (stop_it == stops_.end()) return false;

    auto stop_buses_it = stop_buses_.find(hash);
    if (stop_buses_it == stop_buses_.end() || stop_buses_it->second.empty()) {
        info->stop_buses.clear();
        return true;
    }
    
    info->stop_buses = stop_buses_it->second;
    return true;
}

} // end of namespace transport_catalogue