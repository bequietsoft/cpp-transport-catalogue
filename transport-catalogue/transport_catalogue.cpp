#include <iostream> // DBG

#include "transport_catalogue.h"


namespace transport_catalogue {

// Подсчитывает количество уникальных остановок маршрута
size_t TransportCatalogue::CountUniqueStops(const Route& route) const {
    std::unordered_set<std::string> unique_stops;
    for (const auto& stop_id : route) {
        unique_stops.insert(stop_id);
    }
    return unique_stops.size();
}

// Вычисляет длину маршрута
double TransportCatalogue::CalculateRouteLength(const Route& route) const {
    if (route.size() < 2) return 0.;

    double length = 0.;
    auto previous_index = GetStopIndex(route[0]);
    assert(previous_index.has_value());
    for (size_t i = 1; i < route.size(); ++i) {
        auto current_index = GetStopIndex(route[i]);
        assert(current_index.has_value());
        length += geo::ComputeDistance(
            stops_[*previous_index].location,
            stops_[*current_index].location
        );
        previous_index = current_index;
    }
    return length;
}

// Обновляет индекс stop_buses_
void TransportCatalogue::UpdateStopBuses(const Route& route, const std::string_view bus_id) {
    for (const auto& stop_id : route) {
        auto stop_index_opt = GetStopIndex(stop_id);
        assert(stop_index_opt.has_value());
        StopBuses& stop_buses = stop_buses_index_map_[*stop_index_opt];
        stop_buses.insert(bus_id); // string to string_view
    }
}

// Добавляет остановку в справочник
void TransportCatalogue::AddStop(const std::string_view stop_id, geo::Coordinates location) {
    if (GetStopIndex(stop_id).has_value()) return;
    
    std::string stop_id_str(stop_id);
    stops_.emplace_back(Stop(stop_id_str, location)); 
    
    size_t index = stops_.size() - 1;
    stops_index_map_.try_emplace(std::move(stop_id_str), index);
}

// Доавляет маршрут в справочник
void TransportCatalogue::AddBus(const std::string_view bus_id, const Route& route) {
    if (GetBusIndex(bus_id).has_value()) return;
    
    std::string bus_id_str(bus_id);
    buses_.emplace_back(Bus(bus_id_str, route));
    
    size_t bus_index = buses_.size() - 1;
    buses_index_map_.try_emplace(std::move(bus_id_str), bus_index);

    UpdateStopBuses(route, buses_[bus_index].id);
}

// Возвращает индекс по наименованию
std::optional<size_t> TransportCatalogue::GetIndex(const std::string_view id, const Index& map) const {
    auto it = map.find(std::string(id));
    if (it == map.end()) return std::nullopt;
    return it->second;
}

// Возвращает индекс остановки по наименованию
std::optional<size_t> TransportCatalogue::GetStopIndex(const std::string_view stop_id) const {
    auto index = GetIndex(stop_id, stops_index_map_);
    if (!index.has_value() || *index >= stops_.size()) return std::nullopt;
    return index;
}

// Возвращает индекс маршрута по наименованию
std::optional<size_t> TransportCatalogue::GetBusIndex(const std::string_view bus_id) const {
    auto index = GetIndex(bus_id, buses_index_map_);
    if (!index.has_value() || *index >= buses_.size()) return std::nullopt;
    return index;
}

// Возвращает информацию о маршруте
std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string_view bus_id) const {
    auto bus_index_opt = GetBusIndex(bus_id);
    if (!bus_index_opt.has_value()) return std::nullopt;
    const Bus& bus = buses_.at(*bus_index_opt);

    return BusInfo { 
        bus.route.size(),
        CountUniqueStops(bus.route),
        CalculateRouteLength(bus.route)
    };
}

// Возвращает отсортированный список уникальных маршрутов, проходящих через остановку
std::optional<StopInfo> TransportCatalogue::GetStopInfo(const std::string_view stop_id) const {
    auto stop_index_opt = GetStopIndex(stop_id);
    if(!stop_index_opt.has_value()) {
        // если остановка не найдена в базе возврат nullopt
        return std::nullopt;
    }
    
    auto stop_it = stop_buses_index_map_.find(*stop_index_opt);
    if (stop_it == stop_buses_index_map_.end()) {
        // если индекс отсуствует возвращаем пустой список
        return StopInfo { StopBuses {} };
    }
    
    // возврат копии списка маршрутов остановки
    return StopInfo { 
        stop_it->second // тип StopBuses = std::set<std::string_view>
    };
}

} // end of namespace transport_catalogue