#include "transport_router.h"

namespace routing {

    // Поиск оптимального маршрута 
    std::optional<OptimalRouteInfo> TransportRouter::FindOptimalRoute(
        const TransportCatalogue& db, 
        const std::string_view from_id, const std::string_view to_id) const {

        // 1.0. Получаем указатели на начальную и конечную остановки
        const StopPtr from_ptr = db.GetStop(from_id);
        const StopPtr to_ptr   = db.GetStop(to_id);

        // 1.1. Проверяем существование начальной и конечной остановок
        if (!from_ptr || !to_ptr) {
            return std::nullopt;
        }
        
        // 1.2. Если начальная и конечная остановки совпадают, ехать никуда не нужно
        if (from_ptr == to_ptr) {
            OptimalRouteInfo info;
            info.total_time = 0.0;
            return info;
        }

        // 1.3. Проверяем активность начальной и конечной остановок (через остановки ходят автобусы)
        if (!db.IsActiveStop(from_id) || !db.IsActiveStop(to_id)) {
            return std::nullopt;
        }

        // 2.0. Построение графа маршрутизации (перестроение при добавлении остановок в каталог)
        if (route_graph_.GetVertexCount() < db.GetAllStops().size()) {
            RebuildRouteGraph(db);
        }

        // 3.0. Поиск кратчайшего пути
        graph::Router<double> router(route_graph_);
        auto build_route_opt = router.BuildRoute(db.GetStopIndex(from_ptr), db.GetStopIndex(to_ptr));
        if (!build_route_opt) {
            return std::nullopt;
        }

        // 4.0. Подготовка отчета
        return std::move(MakeOptimalRouteInfo(build_route_opt.value()));
    }


    // Вспомогательные функции FindOptimalRoute:

    // 2.0. Построение графа маршрутизации
    void TransportRouter::RebuildRouteGraph(const TransportCatalogue& db) const {
        assert(routing_settings_.bus_velocity != 0);

        if (route_graph_.GetVertexCount() > 0) {
            route_graph_.Clear();
        }

        route_graph_ = graph::DirectedWeightedGraph<double>(db.GetAllStops().size());
        for (const Bus& bus : db.GetAllBuses()) {
            
            // В маршруте автобуса должно быть как минимум 2 остановки 
            if (bus.route.size() < 2) continue;

            for (size_t i = 0; i < bus.route.size() - 1; ++i) {
                StopPtr from_ptr = bus.route[i];
                StopPtr prev_ptr = from_ptr;

                double accumulate_distance = 0.0;
                for (size_t j = i + 1; j < bus.route.size(); ++j) {
                    StopPtr to_ptr = bus.route[j];

                    accumulate_distance += db.GetDistance(prev_ptr, to_ptr) / 1000;   // расстояние в километрах
                    
                    graph::EdgeId edge_id = AddRouteEdge(
                        db.GetStopIndex(from_ptr), 
                        db.GetStopIndex(to_ptr), 
                        60.0 * accumulate_distance / routing_settings_.bus_velocity); // время в минутах
                        
                    // архивация данных ребра
                    EdgeInfo info;
                        info.wait_stop_id = from_ptr->id;
                        info.bus_id = bus.id;
                        info.span_count = j -i;
                    edges_data_[edge_id] = std::move(info);

                    prev_ptr = to_ptr;
                }
            }
        }    
    }

    // 2.1. Добавление ребра графа
    graph::EdgeId TransportRouter::AddRouteEdge(
        graph::VertexId from_vertex, graph::VertexId to_vertex, double time) const {

        graph::Edge<double> edge;
            edge.from = from_vertex;
            edge.to = to_vertex;
            edge.weight = time + routing_settings_.bus_wait_time; 

        return route_graph_.AddEdge(std::move(edge));
    }

    // 4.0. Подготовка отчёта
    OptimalRouteInfo TransportRouter::MakeOptimalRouteInfo(
        const graph::Router<double>::RouteInfo& route_info) const {

        OptimalRouteInfo info;
        info.total_time = route_info.weight;

        for (const graph::EdgeId edge_id : route_info.edges) {
        
            info.AddWaitAction(
                edges_data_[edge_id].wait_stop_id, 
                routing_settings_.bus_wait_time);

            info.AddBusAction(
                edges_data_[edge_id].bus_id,
                edges_data_[edge_id].span_count, 
                route_graph_.GetEdge(edge_id).weight - routing_settings_.bus_wait_time);
        }

        return info;
    }

} // end of namespace routing