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
        assert(GetStop(stop_id_view) != nullptr);
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
    return {
        bus_ptr->route.size(),
        GetUniqueStopsCount(bus_ptr),
        GetLength(bus_ptr)
    };
}

// Возвращает уникальные остановки маршрута
size_t TransportCatalogue::GetUniqueStopsCount(const Bus* bus_ptr) const {
    assert(bus_ptr != nullptr);
    std::unordered_set<std::string_view> temp_unique_stops; 
    for (auto stop: bus_ptr->route) { 
        temp_unique_stops.insert(stop->id); 
    } 
    return temp_unique_stops.size(); 
}

// Вычисляет длину маршрута
double TransportCatalogue::GetLength(const Bus* bus_ptr) const {
    assert(bus_ptr != nullptr);
    if (bus_ptr->route.size() < 2) return 0.;
    double length = 0.;
    for (size_t i = 1; i < bus_ptr->route.size(); ++i) {
        length += geo::ComputeDistance(
            bus_ptr->route[i    ]->location, 
            bus_ptr->route[i - 1]->location
        );
    }
    return length;
}

} // end of namespace transport_catalogue