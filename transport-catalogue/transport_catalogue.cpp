#include "transport_catalogue.h"


namespace transport_catalogue {

// Методы построения:

// Доавляет маршрут
void TransportCatalogue::AddBus(const std::string_view id, const std::vector<std::string_view>& stops) {
    assert(GetBus(id) == nullptr);
    
    // построение маршрута (vector<Stop*>)
    Route route;
    route.reserve(stops.size());
    for (auto stop_id_view: stops) {
        if (!GetStop(stop_id_view)) AddStop(stop_id_view);
        route.push_back(stops_map_.at(stop_id_view));
    }

    // создание объекта в хранилище (string и vector<Stop*>),
    // move-семантика в инициализаторе объекта Bus
    buses_.emplace_back(std::string(id), route);
    Bus* bus_ptr = &buses_.back();

    // создание индекса на объект (string_view и Bus*)
    std::string_view bus_id_view = bus_ptr->id;
    buses_map_.try_emplace(bus_id_view, bus_ptr);
    
    // обновление списка автобусов остановок
    // формат списка set<Bus*>
    for (auto stop: route) {
        StopBuses* stop_buses_ptr = &stop_buses_map_[stop->id];
        stop_buses_ptr->insert(bus_ptr->id);
    }
}

// Добавляет остановку
void TransportCatalogue::AddStop(const std::string_view id, geo::Coordinates location) {
    assert(GetStop(id) == nullptr);
    
    // создание объекта в хранилище (string и geo::Coordinates),
    // move-семантика в инициализаторе объекта Stop
    stops_.emplace_back(std::string(id), location); 
    Stop* stop_ptr = &stops_.back();

    // создание индекса на объект (string_view и Stop*)
    std::string_view stop_id_view = stop_ptr->id;
    stops_map_.try_emplace(stop_id_view, stop_ptr);

    // инициализация списка автобусов остановки
    stop_buses_map_[stop_id_view] = {};
}

// Добавляет растояние между парой остановок
void TransportCatalogue::AddDistance(const std::string_view from, const std::string_view to, double distance) {
    const Stop* from_stop_ptr = GetStop(from);
    const Stop* to_stop_ptr = GetStop(to);
    if (from_stop_ptr && to_stop_ptr) {
        distances_map_[from_stop_ptr->id][to_stop_ptr->id] = distance;
    }
}

// Методы доступа:

// Возвращает указатель на маршрут
const Bus* TransportCatalogue::GetBus(const std::string_view id) const {
    auto it = buses_map_.find(id);
    return it == buses_map_.end() ? nullptr : it->second;
}

// Возвращает указатель на остановку
const Stop* TransportCatalogue::GetStop(const std::string_view id) const {
    auto it = stops_map_.find(id);
    return it == stops_map_.end() ? nullptr : it->second;
}

// Возвращает растояние между остановками
double TransportCatalogue::GetDistance(const Stop* from_stop_ptr, const Stop* to_stop_ptr) const {
    const auto& from_stop_id = from_stop_ptr->id;
    const auto& to_stop_id = to_stop_ptr->id;
    
    // Поиск предварительно заданного расстояния (прямой):
    auto it_from = distances_map_.find(from_stop_id);
    if (it_from != distances_map_.end()) {
        auto it_to = it_from->second.find(to_stop_id);
        if (it_to != it_from->second.end()) {
            return it_to->second;        
        }
    }

    // Поиск предварительно заданного расстояния (обратный):
    auto it_to = distances_map_.find(to_stop_id);
    if (it_to != distances_map_.end()) {
        auto it_from = it_to->second.find(from_stop_id);
        if (it_from != it_to->second.end()) {
            return it_from->second;        
        }
    }

    // Заданного растояния нет, возврат расстояния между геопозициями остановок:
    return geo::ComputeDistance(from_stop_ptr->location, to_stop_ptr->location);
}

// Методы запросов:

// Возвращает указатель на список маршрутов остановки
const StopBuses* TransportCatalogue::GetStopBuses(const Stop* stop_ptr) const {
    assert(stop_ptr != nullptr);
    auto it = stop_buses_map_.find(stop_ptr->id);
    return it == stop_buses_map_.end() ? nullptr : &it->second;
}

// Возвращает информацию о маршруте
const BusInfo TransportCatalogue::GetBusInfo(const Bus* bus_ptr) const {
    assert(bus_ptr != nullptr);
    
    double fact_distance = GetRouteLength(bus_ptr, false);
    double geo_distance  = GetRouteLength(bus_ptr, true);

    assert(geo_distance != 0.0);
    double curvature = fact_distance / geo_distance;
    
    return {
        bus_ptr->route.size(),
        GetRouteUniqueStopsCount(bus_ptr),
        fact_distance,
        curvature
    };
}

// Возвращает уникальные остановки маршрута
size_t TransportCatalogue::GetRouteUniqueStopsCount(const Bus* bus_ptr) const {
    assert(bus_ptr != nullptr);
    std::unordered_set<std::string_view> temp_unique_stops; 
    for (auto stop: bus_ptr->route) { 
        temp_unique_stops.insert(stop->id); 
    } 
    return temp_unique_stops.size(); 
}

// Вовраает длину маршрута
// use_geo_distance - вычисление по гео-точкам (по умолчанию false)
double TransportCatalogue::GetRouteLength(const Bus* bus_ptr, bool use_geo_distance) const {
    assert(bus_ptr != nullptr);
    if (bus_ptr->route.size() < 2) return 0.;
    
    double distance = 0.;
    for (size_t i = 1; i < bus_ptr->route.size(); ++i) {
        const Stop* from = bus_ptr->route[i - 1];
        const Stop* to = bus_ptr->route[i];
        distance += 
            use_geo_distance ? 
                geo::ComputeDistance(from->location, to->location) :
                GetDistance(from, to);
    }
    
    return distance;
}

} // end of namespace transport_catalogue