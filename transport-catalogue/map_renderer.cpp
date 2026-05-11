#include "map_renderer.h"

namespace render  {

    // Утилиты:
    namespace detail {

        // Вовращает координаты всех остановок маршрутов buses
        std::vector<geo::Coordinates> AllBusesStopsCoords(const Buses& buses) {
            // Получаем количество всех остановок в маршрутах
            size_t total_stops_count = 0;
            for (const auto bus_ptr : buses) {
                total_stops_count += bus_ptr->route.size();
            }
            
            // Собираем координаты остановок
            std::vector<geo::Coordinates> geo_coords;
            geo_coords.reserve(total_stops_count);
            for (const auto bus_ptr : buses) {
                for (const auto stop_ptr : bus_ptr->route) {
                    geo_coords.push_back(stop_ptr->location);
                }
            }

            return geo_coords;
        }

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
            
            svg::Text foreground_label = MapLabel(
                position,
                settings.bus_label_offset,
                settings.bus_label_font_size,
                name,
                {settings.color_palette[route_count % settings.color_palette.size()]},
                0,
                "bold"
            );

            svg::Text underlayer_label = MapLabel(
                position,
                settings.bus_label_offset,
                settings.bus_label_font_size,
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
            
            svg::Text foreground_label = MapLabel(
                position,
                settings.stop_label_offset,
                settings.stop_label_font_size,
                name,
                "black",
                0,
                "normal"
            );

            svg::Text underlayer_label = MapLabel(
                position,
                settings.stop_label_offset,
                settings.stop_label_font_size,
                name,
                {settings.underlayer_color},
                settings.underlayer_width,
                "normal"
            );

            map.Add(std::move(underlayer_label));
            map.Add(std::move(foreground_label));
        }    

    } // end of namespace detail

    // Отрисовка линий маршрутов
    void MapRenderer::RenderLines(svg::Document& map, const Buses& sorted_buses, 
        const std::vector<geo::Coordinates>& geo_coords, const geo::SphereProjector sphere_projector) const {

        size_t route_offset = 0;
        size_t route_count = 0;
        for (const auto bus_ptr : sorted_buses) {
            svg::Polyline poly_line;
            for(size_t i = 0; i < bus_ptr->route.size(); ++i) {
                svg::Point stop_coords = 
                    detail::MapCoords(geo_coords[route_offset + i], sphere_projector);
                poly_line.AddPoint(stop_coords);
            }

            size_t color_index = route_count % render_settings_.color_palette.size();
            svg::Color route_color {render_settings_.color_palette[color_index]};

            poly_line
                .SetStrokeColor(route_color)
                .SetFillColor(svg::NoneColor)
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            map.Add(std::move(poly_line));

            route_offset += bus_ptr->route.size();
            ++route_count;
        }
    }
    
    // Отрисовка названий маршрутов
    void MapRenderer::RenderBusesLabels(svg::Document& map, const Buses& sorted_buses, 
        const std::vector<geo::Coordinates>& geo_coords, const geo::SphereProjector sphere_projector) const {

        size_t route_offset = 0;
        size_t route_count = 0;
        for (const auto bus_ptr : sorted_buses) {            
            // Для всех маршрутов добавляем первую конечную остановку:
            svg::Point position = 
                detail::MapCoords(geo_coords[route_offset], sphere_projector);
            detail::AddBusLabel(map, render_settings_, bus_ptr->id, position, route_count);

            // Если маршрут некольцевой добавляем вторую конечную остановку
            // если она не совпадает с первой конечной остановкой
            if (!bus_ptr->is_roundtrip && bus_ptr->route.size() > 1) {
                size_t end_stop_index = (bus_ptr->route.size() - 1) / 2;
                StopPtr start_stop = bus_ptr->route[0];
                StopPtr end_stop = bus_ptr->route[end_stop_index];
                if (start_stop != end_stop) {
                    svg::Point position = 
                        detail::MapCoords(geo_coords[route_offset + end_stop_index], sphere_projector);
                    detail::AddBusLabel(map, render_settings_, bus_ptr->id, position, route_count);
                }
            }

            route_offset += bus_ptr->route.size();
            ++route_count;
        }
    }

    // Отрисовка остановок
    void MapRenderer::RenderStops(svg::Document& map, const Stops& active_sorted_stops, 
        const geo::SphereProjector sphere_projector) const {

        for(const auto& stop_ptr : active_sorted_stops) {
            svg::Circle stop_sign;
            svg::Point position = 
                detail::MapCoords(stop_ptr->location, sphere_projector);

            stop_sign
                .SetCenter(position)
                .SetRadius(render_settings_.stop_radius)
                .SetFillColor("white");
            map.Add(std::move(stop_sign));
        }        
    }

    // Отрисовка названий остановок
    void MapRenderer::RenderStopsLabels(svg::Document& map, const Stops& active_sorted_stops, 
        const geo::SphereProjector sphere_projector) const {

        for(const auto& stop_ptr : active_sorted_stops) {
            svg::Point position = 
                detail::MapCoords(stop_ptr->location, sphere_projector);
            detail::AddStopLabel(map, render_settings_, stop_ptr->id, position);
        }
    }

    // Отрисовка карты маршрутов в SVG
    svg::Document MapRenderer::RenderMap(const TransportCatalogue& db) const {
        svg::Document map;
        
        // Получаем список всех маршрутов отсортированный по именам
        const Buses sorted_buses = db.GetSortedBuses();

        // Получаем список всех остановок отсортированный по именам
        const Stops active_sorted_stops = db.GetActiveSortedStops();

        // Координаты всех остановок маршрутов
        std::vector<geo::Coordinates> geo_coords = 
            detail::AllBusesStopsCoords(sorted_buses);

        // Проектор сферических координат на карту
        geo::SphereProjector sphere_projector {
            geo_coords.begin(), geo_coords.end(), 
            render_settings_.width, render_settings_.height, render_settings_.padding
        };
        
        // Отрисовка линий маршрутов
        RenderLines(map, sorted_buses, geo_coords, sphere_projector);
        
        // Отрисовка названий маршрутов
        RenderBusesLabels(map, sorted_buses, geo_coords, sphere_projector);

        // Отрисовка остановок
        RenderStops(map, active_sorted_stops, sphere_projector);

        // Отрисовка названий остановок
        RenderStopsLabels(map, active_sorted_stops, sphere_projector);

        return map;
	}

} // end of namespace renderer