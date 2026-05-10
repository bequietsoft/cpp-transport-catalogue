#include "json_reader.h"

namespace transport_catalogue::input {

    // Обрабатывает запросы формирования базы
    void JsonReader::ProcessBaseRequests(json::Document& request_data) {
        const auto& root = request_data.GetRoot().AsMap();

        if (root.count("base_requests")) {
            // Извлекаем массив запросов формирования базы
            const auto& base_requests = root.at("base_requests").AsArray();

            for (const auto& request_node : base_requests) {
                const auto& request = request_node.AsMap();
                const std::string type = request.at("type").AsString();

                if (type == "Stop") ProcessStopRequest(request);
                else
                if (type == "Bus")  ProcessBusRequest(request);
            }
        }
    }

    // Обрабатывает запрос создания остановки
    void JsonReader::ProcessStopRequest(const json::Dict& request) {
        const std::string stop_name = request.at("name").AsString();
        geo::Coordinates location {
            request.at("latitude").AsDouble(), request.at("longitude").AsDouble()
        };
        db_.AddStop(stop_name, location);

        // Обрабатываем расстояния до соседних остановок
        if (request.count("road_distances")) {
            const auto& distances = request.at("road_distances").AsMap();
            for (const auto& [to_stop, distance_node] : distances) {
                double distance = distance_node.AsDouble();
                db_.AddDistance(stop_name, to_stop, distance);
            }
        }
    }

    // Обрабатывает запрос создания маршрута
    void JsonReader::ProcessBusRequest(const json::Dict& request) {
        const std::string name = request.at("name").AsString();
        const auto& stops = request.at("stops").AsArray();
        bool is_roundtrip = request.at("is_roundtrip").AsBool();

        std::vector<std::string_view> route;
        route.reserve(stops.size() + (is_roundtrip ? 0 : stops.size() - 1));

        // Прямой проход (все маршруты)
        for (const auto& stop_node : stops) {
            std::string_view stop_name = stop_node.AsString();
            route.push_back(stop_name);
        }
    
        // Обратный проход для некольцевого
        if (!is_roundtrip && stops.size() > 1) {
            for (size_t i = stops.size() - 2; i != static_cast<size_t>(-1); --i) {
                std::string_view stop_name = stops[i].AsString();
                route.push_back(stop_name);
            }
        }

        db_.AddBus(name, route, is_roundtrip);
    }

} // end of namespace transport_catalogue::input