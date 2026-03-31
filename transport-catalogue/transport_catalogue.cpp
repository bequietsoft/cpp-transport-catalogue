#include "transport_catalogue.h"


namespace transport_catalogue {

// Методы построения:

// Доавляет маршрут
void TransportCatalogue::AddBus(const std::string_view id, const std::vector<std::string_view>& stops) {
    Bus* bus_ptr = GetBus(id); 
    assert(bus_ptr == nullptr);
    
    // построение маршрута (vector<Stop*>)
    Route route;
    route.reserve(stops.size());
    for (auto id: stops) {
        Stop* stop_ptr = GetStop(id);
        assert(stop_ptr != nullptr);
        route.emplace_back(stop_ptr);
    }

    // создание объекта в хранилище (string и vector<Stop*>),
    // move-семантика в инициализаторе объекта Bus
    buses_.emplace_back(Bus(std::string(id), route));
    bus_ptr = &buses_.back();

    // создание индекса на объект (string_view и Bus*)
    std::string_view bus_id_view = bus_ptr->id;
    buses_map_.try_emplace(bus_id_view, bus_ptr);
    
    // обновление списка автобусов остановки на маршруте
    // set<std::string_view>
    for (auto stop : bus_ptr->route) {
        stop->buses.insert(bus_id_view);
    }
}

// Добавляет остановку
void TransportCatalogue::AddStop(const std::string_view id, geo::Coordinates location) {
    Stop* stop_ptr = GetStop(id);
    assert(stop_ptr == nullptr);

    // создание объекта в хранилище (string и geo::Coordinates),
    // move-семантика в инициализаторе объекта Stop
    stops_.emplace_back(Stop(std::string(id), location)); 
    stop_ptr = &stops_.back();

    // создание индекса на объект (string_view и Stop*)
    std::string_view stop_id_view = stop_ptr->id;
    stops_map_.try_emplace(stop_id_view, stop_ptr);
}

// Методы доступа:

// Возвращает указатель на маршрут
Bus* TransportCatalogue::GetBus(const std::string_view id) const {
    auto it = buses_map_.find(id);
    return it == buses_map_.end() ? nullptr : it->second;
}

// Возвращает указатель на остановку
Stop* TransportCatalogue::GetStop(const std::string_view id) const {
    auto it = stops_map_.find(id);
    return it == stops_map_.end() ? nullptr : it->second;
}

// Вспомогательные методы:

// Подсчитывает количество уникальных остановок маршрута
size_t TransportCatalogue::GetRouteUniqueStopsCount(const Route& route) const {
    std::unordered_set<std::string_view> temp_unique_stops;
    for (auto stop: route) {
        temp_unique_stops.insert(stop->id);
    }
    return temp_unique_stops.size();
}

// Вычисляет длину маршрута
double TransportCatalogue::GetRouteLength(const Route& route) const {
    if (route.size() < 2) return 0.;
    double length = 0.;
    for (size_t i = 1; i < route.size(); ++i) {
        length += geo::ComputeDistance(route[i]->location, route[i - 1]->location);
    }
    return length;
}

} // end of namespace transport_catalogue