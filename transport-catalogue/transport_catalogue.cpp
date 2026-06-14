#include "transport_catalogue.h"

namespace transport_catalogue {

    // Методы построения:

    // Добавляет автобус
    BusPtr TransportCatalogue::AddBus(const std::string_view id, 
        const std::vector<std::string_view>& stops_ids, bool is_roundtrip) {
            
        // Автобус существует
        assert(GetBus(id) == nullptr);

        // Остановки автобуса (Stops = vector<StopPtr>)
        Stops stops;
        stops.reserve(stops_ids.size());
        for (auto stop_id: stops_ids) {
            if (StopPtr stop_ptr = AddStop(stop_id, {}))
                stops.push_back(stop_ptr);
        }

        // Создание объекта в хранилище (string и vector<StopPtr>),
        // move-семантика в инициализаторе объекта Bus
        Bus bus(std::string(id), stops, is_roundtrip);
        buses_.emplace_back(std::move(bus));
        BusPtr bus_ptr = &buses_.back();

        // Создание индекса на объект (string_view и Bus*)
        std::string_view bus_id_view = bus_ptr->id;
        buses_map_.try_emplace(bus_id_view, bus_ptr);
        
        // Обновление списка автобусов остановок stops
        // формат (set<BusPtr> c лямбда-компаратором лексикографической сортировки)
        for (auto stop: stops) {
            StopBuses& stop_buses = stop_buses_map_[stop->id];
            stop_buses.insert(bus_ptr);
        }

        return bus_ptr;
    }

    // Добавляет остановку
    StopPtr TransportCatalogue::AddStop(const std::string_view id, geo::Coordinates location) {
        // Проверка существования остановки
        StopPtr exist_stop_ptr = GetStop(id);
        if (exist_stop_ptr) {
            // Обновление геолокации, если заданы новые координаты
            if (!std::isnan(location.lat) && !std::isnan(location.lng)) {
                exist_stop_ptr->location = location;
            }
            return exist_stop_ptr;
        }

        // Cоздание объекта в хранилище (string и geo::Coordinates),
        // move-семантика в инициализаторе объекта Stop
        stops_.emplace_back(std::string(id), location); 
        StopPtr stop_ptr = &stops_.back();

        // Cоздание индекса на объект (string_view и Stop*)
        std::string_view stop_id_view = stop_ptr->id;
        stops_map_.try_emplace(stop_id_view, stop_ptr);

        stops_ids_map_.try_emplace(stop_ptr, stops_.size() - 1);

        // Инициализация списка автобусов остановки
        stop_buses_map_[stop_id_view] = {};

        return stop_ptr;
    }

    // Добавляет растояние между парой остановок
    void TransportCatalogue::AddDistance(const std::string_view from, const std::string_view to, double distance) {
        StopPtr from_stop_ptr = AddStop(from, {});
        StopPtr to_stop_ptr = AddStop(to, {});
        distances_map_[from_stop_ptr->id][to_stop_ptr->id] = distance;
    }


    // Методы доступа:

    // Возвращает указатель на автобус
    BusPtr TransportCatalogue::GetBus(const std::string_view id) const {
        auto it = buses_map_.find(id);
        return it == buses_map_.end() ? nullptr : it->second;
    }

    // Возвращает указатель на остановку
    StopPtr TransportCatalogue::GetStop(const std::string_view id) const {
        auto it = stops_map_.find(id);
        return it == stops_map_.end() ? nullptr : it->second;
    }

    // Возвращает растояние между остановками
    double TransportCatalogue::GetDistance(const StopPtr from_stop_ptr, const StopPtr to_stop_ptr) const {
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
        double compute_distance = geo::ComputeDistance(from_stop_ptr->location, to_stop_ptr->location);
        
        // Запомнить расстояние, если нет заранее заданного или вычисленного
        distances_map_[from_stop_ptr->id][to_stop_ptr->id] = compute_distance;
        distances_map_[to_stop_ptr->id][from_stop_ptr->id] = compute_distance;

        return compute_distance;
    }


    // Возвращает список всех остановок (чтение)
    const std::deque<Stop>& TransportCatalogue::GetAllStops() const {
        return stops_;
    }

    // Возвращает список всех автобусов (чтение)
    const std::deque<Bus>& TransportCatalogue::GetAllBuses() const {
        return buses_;
    }

    // Возвращает индекс остановки в stops_
    size_t TransportCatalogue::GetStopIndex(StopPtr stop_ptr) const {
        size_t index = stops_ids_map_.at(stop_ptr);
        return index;
    }
    

    // Методы запросов:

    // Возвращает указатель на список автобусов остановки
    const StopBuses* TransportCatalogue::GetBusesByStop(const StopPtr stop_ptr) const {
        assert(stop_ptr != nullptr);
        auto it = stop_buses_map_.find(stop_ptr->id);
        return it == stop_buses_map_.end() ? nullptr : &it->second;
    }

    // Возвращает информацию об автобусе
    const BusStat TransportCatalogue::GetBusStat(const BusPtr bus_ptr) const {
        assert(bus_ptr != nullptr);
        
        double fact_distance = GetRouteLength(bus_ptr, false);
        double geo_distance  = GetRouteLength(bus_ptr, true);
        
        assert(geo_distance != 0.0);
        double curvature = fact_distance / geo_distance;
        
        return {
            bus_ptr->route.size(),              // stop_count
            GetRouteUniqueStopsCount(bus_ptr),  // unique_stop_count
            fact_distance,                      // route_length
            curvature                           // curvature
        };
    }

    // Возвращает уникальные остановки автобуса
    size_t TransportCatalogue::GetRouteUniqueStopsCount(const BusPtr bus_ptr) const {
        assert(bus_ptr != nullptr);

        std::vector<std::string_view> temp;
        temp.reserve(bus_ptr->route.size());
        for (const auto stop_ptr : bus_ptr->route)
            temp.emplace_back(stop_ptr->id);

        std::sort(temp.begin(), temp.end());
        auto first_double = std::unique(temp.begin(), temp.end());

        return std::distance(temp.begin(), first_double);
    }

    // Возвращает длину маршрута автобуса
    // use_geo_distance - вычисление по гео-точкам (по умолчанию false)
    double TransportCatalogue::GetRouteLength(const BusPtr bus_ptr, bool use_geo_distance) const {
        assert(bus_ptr != nullptr);

        if (bus_ptr->route.size() < 2) return 0.;
        
        double distance = 0.;
        for (size_t i = 1; i < bus_ptr->route.size(); ++i) {
            const StopPtr from = bus_ptr->route[i - 1];
            const StopPtr to = bus_ptr->route[i];
            distance += 
                use_geo_distance ? 
                    geo::ComputeDistance(from->location, to->location) :
                    GetDistance(from, to);
        }
        
        return distance;
    }


    namespace detail {

        // Возвращает отсортированный вектор элементов индекс-карты 
        template<typename K, typename V, typename Comparator>
        std::vector<V> SortedMap(
            const std::unordered_map<K, V>& data, Comparator comparator) {
            
            std::vector<V> temp;
            temp.reserve(data.size());
            
            for (const auto& [id, item] : data) 
                temp.push_back(item);
            
            std::sort(temp.begin(), temp.end(), comparator);
            return temp;
        }

    } // end of namespace detail
    
    // Возвращает отсортированный вектор автобусов
    const Buses TransportCatalogue::GetSortedBuses() const {
        return detail::SortedMap<std::string_view, BusPtr>(
            buses_map_,     
            [](const BusPtr a, const BusPtr b) {
                return a->id < b->id;
            }
        );
    }

    // Возвращает отсортированный вектор остановок, через которые ходят автобусы
    const Stops TransportCatalogue::GetActiveSortedStops() const {

        std::unordered_map<std::string_view, StopPtr> active_stops_map_;
        for(const auto& [stop_id_view, stop_ptr] : stops_map_) {
            const StopBuses* stop_buses = GetBusesByStop(stop_ptr);
            if (stop_buses && !stop_buses->empty())
                active_stops_map_.try_emplace(stop_id_view, stop_ptr);
        }

        return detail::SortedMap<std::string_view, StopPtr>(
            active_stops_map_,
            [](const StopPtr a, const StopPtr b) {
                return a->id < b->id;
            }
        );
    }

    // Активная остановка (через остановку ходят автобусы)
    bool TransportCatalogue::IsActiveStop(const std::string_view id) const {
        auto it = stop_buses_map_.find(id);
        return it == stop_buses_map_.end() ? false : static_cast<bool>(it->second.size());
    }

} // end of namespace transport_catalogue