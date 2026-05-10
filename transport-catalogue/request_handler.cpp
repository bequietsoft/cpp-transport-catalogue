#include "request_handler.h"

namespace transport_catalogue::output {
    
    using namespace transport_catalogue;

    // Утилиты:
    namespace detail {

        // Возвращает положение объекта на карте по гео-координате
        svg::Point MapCoords(
            const geo::Coordinates& geo_coord, const geo::SphereProjector& sphere_projector) {
            geo::PlaneCoordinates plane_coord = sphere_projector(geo_coord);
            return {
                plane_coord.x, plane_coord.y
            };
        }

        // Формирование объекта стилизованной надписи
        svg::Text MapLabel(
            const svg::Point& position, const svg::Point& offset, int size, const std::string& data, 
            const svg::Color& color, double underlayer_width, const std::string& font_weight) {
            
            svg::Text label;

            label
                .SetFillColor(color)
                .SetPosition(position)
                .SetOffset(offset)
                .SetFontSize(size)
                .SetFontFamily("Verdana")
                .SetData(data);

            if (font_weight == "bold") 
                label.SetFontWeight(font_weight);

            if (underlayer_width != 0) {
                label
                    .SetStrokeColor(color)
                    .SetStrokeWidth(underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            }

            return label;
        }

        // Добавляет заголовок маршрута на карту
        void AddBusLabel(svg::Document& map, const render::RenderSettings& settings,
            const std::string& name, const svg::Point& position, size_t route_count) {
            
            const svg::Point& offset = { settings.bus_label_offset.x, settings.bus_label_offset.y };
            const int font_size = settings.bus_label_font_size;
            const size_t color_index = route_count % settings.color_palette.size();

            svg::Text foreground_label = MapLabel(
                position,
                offset,
                font_size,
                name,
                {settings.color_palette[color_index]},
                0,
                "bold"
            );

            svg::Text underlayer_label = MapLabel(
                position,
                offset,
                font_size,
                name,
                {settings.underlayer_color},
                settings.underlayer_width,
                "bold"
            );

            map.Add(std::move(underlayer_label));
            map.Add(std::move(foreground_label));
        }

        // Добавляет заголовок остановки на карту
        void AddStopLabel(svg::Document& map, const render::RenderSettings& settings,
            const std::string& name, const svg::Point& position) {
            
            const svg::Point& offset  = { settings.stop_label_offset.x, settings.stop_label_offset.y };
            const int font_size = settings.stop_label_font_size;
        
            svg::Text foreground_label = MapLabel(
                position,
                offset,
                font_size,
                name,
                "black",
                0,
                "normal"
            );

            svg::Text underlayer_label = MapLabel(
                position,
                offset,
                font_size,
                name,
                {settings.underlayer_color},
                settings.underlayer_width,
                "normal"
            );

            map.Add(std::move(underlayer_label));
            map.Add(std::move(foreground_label));
        }    

    } // end of namespace detail

    // Обработка запросов к базе
    json::Document RequestHandler::ProcessStatRequests(json::Document& request_data) const {
        json::Array response_array; 
        const auto& root = request_data.GetRoot().AsMap();

        if (root.count("stat_requests")) {
            const auto& stat_requests = root.at("stat_requests").AsArray();

            for (const auto& request_node : stat_requests) {
                const auto& request = request_node.AsMap();

                int request_id = request.at("id").AsInt();
                std::string_view request_type = request.at("type").AsString();

                // Объект для одного ответа
                json::Dict response_item; 
                response_item["request_id"] = json::Node(request_id);

                // Запрос информации об остановке
                if (request_type == "Stop") {
                    std::string_view stop_name = request.at("name").AsString();
                    const StopBuses* stop_buses_ptr = GetBusesByStop(stop_name);
                    if (stop_buses_ptr) {
                        json::Array buses_array;
                        for (auto bus_ptr : *stop_buses_ptr) {
                            buses_array.push_back(json::Node(std::string(bus_ptr->id)));
                        }
                        response_item["buses"] = json::Node(buses_array);
                    }
                    else {
                        response_item["error_message"] = json::Node(std::string("not found"));
                    }
                } 
                
                // Запрос информации о маршруте
                if (request_type == "Bus") {
                    std::string_view bus_name = request.at("name").AsString();
                    std::optional<BusStat> bus_stat_opt = GetBusStat(bus_name);
                    if (bus_stat_opt.has_value()) {
                        const BusStat& stat = bus_stat_opt.value();
                        response_item["curvature"] = json::Node(static_cast<double>(stat.curvature));
                        response_item["route_length"] = json::Node(static_cast<double>(stat.route_length));
                        response_item["stop_count"] = json::Node(static_cast<int>(stat.stop_count));
                        response_item["unique_stop_count"] = json::Node(static_cast<int>(stat.unique_stop_count));
                    }
                    else {
                        response_item["error_message"] = json::Node(std::string("not found"));
                    }
                }

                // Запрос карты маршрута
                if (request_type == "Map") {
                    svg::Document map = RenderMap();
                    std::stringstream string_stream;
                    map.Render(string_stream);
                    response_item["map"] = json::Node(string_stream.str()); // TODO: подавать поток на вход
                }

                // Внести ответ в массив
                json::Node node(response_item);
                response_array.push_back(node);
            }
        }

        json::Node response(response_array);
        return json::Document(std::move(response));
    }

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        const BusPtr bus_ptr = db_.GetBus(bus_name);
        if (!bus_ptr) return std::nullopt;
        return std::move(db_.GetBusStat(bus_ptr));
    }

    // Возвращает маршруты, проходящие через остановку (запрос Stop)
    const StopBuses* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        const StopPtr stop_ptr = db_.GetStop(stop_name);
        if (!stop_ptr) return nullptr;
        return db_.GetBusesByStop(stop_ptr);
    }


    // Отрисовка карты маршрута в SVG
    svg::Document RequestHandler::RenderMap() const {
        svg::Document map;
        render::RenderSettings settings = renderer_.GetRenderSettings();
        
        // Получаем список всех маршрутов отсортированный по именам
        const std::vector<BusPtr> sorted_buses = db_.GetSortedBuses();

        // Получаем количество всех остановок во всех маршрутах
        size_t total_stops_count = 0;
        for (const auto bus_ptr : sorted_buses) 
            total_stops_count += bus_ptr->route.size();
        
        // Собираем координаты всех остановок в один вектор
        std::vector<geo::Coordinates> geo_coords;
        geo_coords.reserve(total_stops_count);
        for (const auto bus_ptr : sorted_buses) {
            for (const auto stop_ptr : bus_ptr->route) {
                geo_coords.push_back(stop_ptr->location);
            }
        }

        // Создаём проектор сферических координат на карту
        geo::SphereProjector sphere_projector {
            geo_coords.begin(), geo_coords.end(), 
            settings.width, settings.height, settings.padding
        };
        
        // 0. Линии маршрутов
        size_t route_offset = 0;
        size_t route_count = 0;
        for (const auto bus_ptr : sorted_buses) {
            svg::Polyline poly_line;
            for(size_t i = 0; i < bus_ptr->route.size(); ++i) {
                svg::Point stop_coords = 
                    detail::MapCoords(geo_coords[route_offset + i], sphere_projector);
                poly_line.AddPoint(stop_coords);
            }

            size_t color_index = route_count % settings.color_palette.size();
            svg::Color route_color {settings.color_palette[color_index]};

            poly_line
                .SetStrokeColor(route_color)
                .SetFillColor(svg::NoneColor)
                .SetStrokeWidth(settings.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            map.Add(std::move(poly_line));

            route_offset += bus_ptr->route.size();
            ++route_count;
        }

        // 1. Названия маршрутов
        route_offset = 0;
        route_count = 0;
        for (const auto bus_ptr : sorted_buses) {            
            // Для всех маршрутов добавляем первую конечную остановку:
            svg::Point position = 
                detail::MapCoords(geo_coords[route_offset], sphere_projector);
            detail::AddBusLabel(map, settings, bus_ptr->id, position, route_count);

            // Если маршрут некольцевой добавляем вторую конечную остановку
            // если она не совпадает с первой конечной остановкой
            if (!bus_ptr->is_roundtrip && bus_ptr->route.size() > 1) {
                size_t end_stop_index = (bus_ptr->route.size() - 1) / 2;
                StopPtr start_stop = bus_ptr->route[0];
                StopPtr end_stop = bus_ptr->route[end_stop_index];
                if (start_stop != end_stop) {
                    svg::Point position = 
                        detail::MapCoords(geo_coords[route_offset + end_stop_index], sphere_projector);
                    detail::AddBusLabel(map, settings, bus_ptr->id, position, route_count);
                }
            }

            route_offset += bus_ptr->route.size();
            ++route_count;
        }

        // 2. Точки остановок
        // Получаем список всех остановок отсортированный по именам
        const std::vector<StopPtr> sorted_stops = db_.GetSortedStops();
        for(const auto& stop_ptr : sorted_stops) {
            // Пропускаем остановки, не включенные ни в один в маршрут
            const StopBuses* stop_buses = db_.GetBusesByStop(stop_ptr);
            if (!stop_buses || stop_buses->empty()) continue;

            svg::Circle stop_sign;
            svg::Point position = 
                detail::MapCoords(stop_ptr->location, sphere_projector);

            stop_sign
                .SetCenter(position)
                .SetRadius(settings.stop_radius)
                .SetFillColor("white");
            map.Add(std::move(stop_sign));
        }

        // 3. Названия остановок
        for(const auto& stop_ptr : sorted_stops) {
            // Пропускаем остановки, не включенные ни в один в маршрут
            const StopBuses* stop_buses = db_.GetBusesByStop(stop_ptr);
            if (!stop_buses || stop_buses->empty()) continue;

            svg::Point position = 
                detail::MapCoords(stop_ptr->location, sphere_projector);
            detail::AddStopLabel(map, settings, stop_ptr->id, position);
        }

        return map; // copy elision on return local object
	}

} // end of namespace transport_catalogue::output