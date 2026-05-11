#include "json_reader.h"

namespace transport_catalogue::input {

    // Утилиты
    namespace detail {

        // Обрабатывает запрос создания остановки
        void ProcessStopRequest(TransportCatalogue& db, const json::Dict& request) {
            const std::string stop_name = request.at("name").AsString();
            geo::Coordinates location {
                request.at("latitude").AsDouble(), request.at("longitude").AsDouble()
            };
            db.AddStop(stop_name, location);

            // Обрабатываем расстояния до соседних остановок
            if (request.count("road_distances")) {
                const auto& distances = request.at("road_distances").AsMap();
                for (const auto& [to_stop, distance_node] : distances) {
                    double distance = distance_node.AsDouble();
                    db.AddDistance(stop_name, to_stop, distance);
                }
            }
        }

        // Обрабатывает запрос создания маршрута
        void ProcessBusRequest(TransportCatalogue& db, const json::Dict& request) {
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

            db.AddBus(name, route, is_roundtrip);
        }

        // Конвертирует узел json::Node в svg::Color
        svg::Color NodeToColor(const json::Node& node) {
            
            // Цвет задан строкой
            if (node.IsString()) {
                return svg::Color(node.AsString());
            } 

            // Цвет задан массивом
            if (node.IsArray()) {
                const auto& color_array = node.AsArray();
                
                // Формат RGB
                if (color_array.size() == 3) {
                    return svg::Color(svg::Rgb{
                        static_cast<uint8_t>(color_array[0].AsInt()),
                        static_cast<uint8_t>(color_array[1].AsInt()),
                        static_cast<uint8_t>(color_array[2].AsInt())
                    });
                } 

                // Формат RGBA
                if (color_array.size() == 4) {
                    return svg::Color(svg::Rgba{
                        static_cast<uint8_t>(color_array[0].AsInt()),
                        static_cast<uint8_t>(color_array[1].AsInt()),
                        static_cast<uint8_t>(color_array[2].AsInt()),
                        color_array[3].AsDouble()
                    });
                }
            }

            // По умолчанию возвращает чёрный цвет
            return svg::Color(svg::Rgb{0, 0, 0});
        }
    
    } // end of namespace detail

    // Обрабатывает запросы формирования базы
    TransportCatalogue JsonReader::GetDataBase(const json::Document& request_data) const {
        TransportCatalogue db {};

        const auto& root = request_data.GetRoot().AsMap();

        if (root.count("base_requests")) {
            // Извлекаем массив запросов
            const auto& base_requests = root.at("base_requests").AsArray();

            for (const auto& request_node : base_requests) {
                const auto& request_dict = request_node.AsMap();
                const std::string type = request_dict.at("type").AsString();

                if (type == "Stop") detail::ProcessStopRequest(db, request_dict);
                if (type == "Bus")  detail::ProcessBusRequest(db, request_dict);
            }

            return db;
        }

        return {};
    }

    // Читает параметры отрисовки
    RenderSettings JsonReader::GetRenderSettings(const json::Document& request_data) const {
        const auto& root = request_data.GetRoot().AsMap();
        
        if (root.count("render_settings")) {
            RenderSettings render_settings;

            const auto& render_settings_node = root.at("render_settings").AsMap();

            render_settings.width = render_settings_node.at("width").AsDouble();
            render_settings.height = render_settings_node.at("height").AsDouble();
            render_settings.padding = render_settings_node.at("padding").AsDouble();

            render_settings.line_width = render_settings_node.at("line_width").AsDouble();
            render_settings.stop_radius = render_settings_node.at("stop_radius").AsDouble();
            
            render_settings.bus_label_font_size = render_settings_node.at("bus_label_font_size").AsInt();
            const auto& bus_label_offset = render_settings_node.at("bus_label_offset").AsArray();
            render_settings.bus_label_offset = { 
                bus_label_offset[0].AsDouble(), 
                bus_label_offset[1].AsDouble()
            };
            
            render_settings.stop_label_font_size = render_settings_node.at("stop_label_font_size").AsInt();
            const auto& stop_label_offset = render_settings_node.at("stop_label_offset").AsArray();
            render_settings.stop_label_offset = { 
                stop_label_offset[0].AsDouble(), 
                stop_label_offset[1].AsDouble()
            };
            
            const auto& underlayer_color = render_settings_node.at("underlayer_color");
            render_settings.underlayer_color = detail::NodeToColor(underlayer_color); 
            render_settings.underlayer_width = render_settings_node.at("underlayer_width").AsDouble();
            
            const auto& color_palette = render_settings_node.at("color_palette").AsArray();
            for(const auto& color_node : color_palette)
                render_settings.color_palette.push_back(detail::NodeToColor(color_node));
            
            return render_settings;
        }

        return {};
    }
    
} // end of namespace transport_catalogue::input