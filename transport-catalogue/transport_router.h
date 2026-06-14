#pragma once

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
//#include <cassert>

namespace routing {

    using namespace transport_catalogue;

    // Параметры маршрутизации
    struct RoutingSettings {
        RoutingSettings() = default;
        int bus_wait_time;                // время ожидания автобуса на остановке, в минутах
        double bus_velocity;              // скорость автобуса, в км/ч
    };

    // Данные ребра
    struct EdgeInfo {
        std::string_view wait_stop_id;    // Название остановки
        std::string_view bus_id;          // Название автобуса
        size_t span_count;                // Количество остановок, которые проезжаем на автобусе
    };
    
    using EdgesData = std::unordered_map<graph::EdgeId, EdgeInfo>;

    // Действие: ожидание на остановке
    struct WaitAction {
        std::string_view stop_id;         // Название остановки
        double time;                      // Время ожидания в минутах (*всегда равно bus_wait_time)
    };

    // Действие: поездка на автобусе
    struct BusAction {
        std::string_view bus_id;          // Название автобуса
        size_t span_count;                // Количество остановок, которые проезжаем на автобусе
        double time;                      // Время в пути на этом участке (без учёта времени ожидания) в минутах
    };

    // Вариант для всех типов действий
    using RouteAction = std::variant<WaitAction, BusAction>;

    // Информация о найденом маршруте
    struct OptimalRouteInfo {
        double total_time;                // Общее время маршрута в минутах
        std::vector<RouteAction> actions; // Последовательность действий (ожидания и поездки)

        void AddWaitAction(const std::string_view stop_name, double wait_time) {
            actions.emplace_back(WaitAction{stop_name, wait_time});
        }

        void AddBusAction(const std::string_view bus_name, size_t stops_count, double ride_time) {
            actions.emplace_back(BusAction{bus_name, stops_count, ride_time});
        }
    };

    // Транспортный маршутизатор
    class TransportRouter {
    public:
        // Конструктор
        explicit TransportRouter(
            const TransportCatalogue& db, const RoutingSettings& routing_settings) 
            : db_(db)
            , routing_settings_(std::move(routing_settings)) {
            
            // 0.0. Построение графа маршрутизации
            BuildRouteGraph();
        }


        // Поиск оптимального маршрута 
        std::optional<OptimalRouteInfo> FindOptimalRoute( 
            const std::string_view from_id, const std::string_view to_id) const;

    private:
        const TransportCatalogue& db_;
        const RoutingSettings routing_settings_;

        mutable graph::DirectedWeightedGraph<double> route_graph_;
        mutable EdgesData edges_data_;
        

        // Вспомогательные функции:

        // 0.0. Построение графа маршрутизации
        void BuildRouteGraph() const;

        // 0.1. Добавление ребра графа
        graph::EdgeId AddRouteEdge(
            graph::VertexId from_vertex, graph::VertexId to_vertex, double time) const;

        // 3.0. Подготовка отчета
        OptimalRouteInfo MakeOptimalRouteInfo(
            const graph::Router<double>::RouteInfo& route_info) const;
    };

} // end of namespace routing