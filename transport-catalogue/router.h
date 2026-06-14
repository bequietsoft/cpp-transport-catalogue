#pragma once

#include "graph.h"

#include <queue>
#include <vector>
#include <optional>
#include <limits>

namespace graph {

    template <typename Weight>
    class Router {
    private:
        using Graph = DirectedWeightedGraph<Weight>;

    public:
        explicit Router(const Graph& graph) : graph_(graph) {}

        struct RouteInfo {
            Weight weight;
            std::vector<EdgeId> edges;
        };

        // Алгоритм A*
        std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const {
            // Обработка случая, когда начальная и конечная вершины совпадают
            if (from == to) return RouteInfo{Weight{0}, {}};

            const size_t vertex_count = graph_.GetVertexCount();

            // Проверка корректности входных вершин
            if (from >= vertex_count || to >= vertex_count) return std::nullopt;

            std::vector<Weight> distances(vertex_count, std::numeric_limits<Weight>::max());
            std::vector<std::optional<EdgeId>> prev_edges(vertex_count);
            std::priority_queue<
                std::pair<Weight, VertexId>,
                std::vector<std::pair<Weight, VertexId>>,
                std::greater<>
            > pq;

            distances[from] = 0;
            pq.push({0, from});

            while (!pq.empty()) {
                auto [current_dist, current_vertex] = pq.top();
                pq.pop();

                // Пропускаем устаревшие записи в очереди
                if (current_dist > distances[current_vertex]) continue;

                // Если достигли целевой вершины, прерываем поиск
                if (current_vertex == to) break;

                // Обход всех исходящих рёбер текущей вершины
                for (EdgeId edge_id : graph_.GetIncidentEdges(current_vertex)) {
                    const auto& edge = graph_.GetEdge(edge_id);

                    // Проверка корректности индекса целевой вершины
                    if (edge.to >= vertex_count) continue;

                    Weight new_dist = distances[current_vertex] + edge.weight;

                    // Если найден более короткий путь, обновляем данные
                    if (new_dist < distances[edge.to]) {
                        distances[edge.to] = new_dist;
                        prev_edges[edge.to] = edge_id;
                        pq.push({new_dist, edge.to});
                    }
                }
            }

            // Если путь не найден, возвращаем nullopt
            if (distances[to] == std::numeric_limits<Weight>::max()) {
                return std::nullopt;
            }

            // Восстановление пути от конечной вершины к начальной
            std::vector<EdgeId> edges;
            VertexId current = to;

            while (prev_edges[current].has_value()) {
                EdgeId edge_id = *prev_edges[current];
                edges.push_back(edge_id);
                current = graph_.GetEdge(edge_id).from;

                // Защита от зацикливания
                if (current == from) break;
            }

            // Разворачиваем путь от from к to
            std::reverse(edges.begin(), edges.end());

            return RouteInfo{distances[to], std::move(edges)};
        }

    private:
        const Graph& graph_;
    };

} // end of namespace graph
