#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cassert>

#include "geo.h"

namespace transport_catalogue {

using Route = std::vector<std::string>;
using Buses = std::set<std::string>;

struct Stop {
	std::string id;
	geo::Coordinates location;

    Stop(const std::string& id, geo::Coordinates location)
        : id(id), location(location) {}
};

struct Bus {
	std::string id;
	Route route;

    Bus(const std::string& id, Route route)
        : id(id), route(std::move(route)) {}
};

struct BusInfo {
	size_t stops_count;
	size_t unique_stops_count;
	double length;
};

struct StopInfo {
	Buses stop_buses;
};

class TransportCatalogue {
public:
	// Добавляет остановку в справочник
	void AddStop(const std::string& id, geo::Coordinates location);

	// Доавляет маршрут в справочник
	void AddBus(const std::string& id, Route& route);
    
	// Возвращает информацию о маршруте
	bool GetBusInfo(const std::string& id, BusInfo* info) const;

	// Возвращает отсортированный список уникальных маршрутов, проходящих через остановку
	bool GetStopInfo(const std::string& id, StopInfo* info) const;
	
private:
	// Массивы данных
    std::unordered_map<size_t, Stop> stops_;
    std::unordered_map<size_t, Bus>  buses_;
	
	// Индекс остановка - маршруты (обновляется при добавлении и изменении маршрутов)
	std::unordered_map<size_t, Buses> stop_buses_;

	// Возвращает хэш строки
	size_t Hash(const std::string& str) const;

	// Подсчитывает количество уникальных остановок маршрута
    size_t CountUniqueStops(const Route& route) const;

	// Вычисляет длину маршрута
    double CalculateRouteLength(const Route& route) const;

	// Обновляет индекс stop_buses_
	void UpdateStopBuses(const Bus& bus);
};

} // end of namespace transport_catalogue