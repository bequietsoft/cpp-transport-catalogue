#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include "transport_catalogue.h"

#include <vector>

namespace render {

    using namespace transport_catalogue;

    struct RenderSettings {
        RenderSettings() = default;

        double width;                            // ширина изображения в пикселях
        double height;                           // высота изображения в пикселях
        double padding;                          //  отступ краёв карты от границ SVG-документа

        double line_width;                       // толщина линий, которыми рисуются автобусные маршруты
        double stop_radius;                      // радиус окружностей, которыми обозначаются остановки

        int bus_label_font_size;                 // размер текста названий автобусных маршрутов
        svg::Point bus_label_offset;             // смещение надписи с названием маршрута относительно координат конечной остановки на карте

        int stop_label_font_size;                // размер текста названий остановок
        svg::Point stop_label_offset;            // смещение названия остановки относительно её координат на карте

        svg::Color underlayer_color;             // цвет подложки под названиями остановок и маршрутов
        double underlayer_width;                 // толщина подложки под названиями остановок и маршрутов

        std::vector<svg::Color> color_palette;   // цветовая палитра (непустой массив)
    };

    class MapRenderer {
    public:
        // Конструктор
        explicit MapRenderer(RenderSettings& render_settings)
            : render_settings_(std::move(render_settings)) {}

        // Отрисовка карты маршрутов в SVG
        svg::Document RenderMap(const TransportCatalogue& db) const;

    private:
        RenderSettings render_settings_ {};

        // Отрисовка линий маршрутов
        void RenderLines(svg::Document& map, const Buses& sorted_buses, 
            const std::vector<geo::Coordinates>& geo_coords, 
            const geo::SphereProjector sphere_projector) const;

        // Отрисовка названий маршрутов
        void RenderBusesLabels(svg::Document& map, const Buses& sorted_buses, 
            const std::vector<geo::Coordinates>& geo_coords, 
            const geo::SphereProjector sphere_projector) const;

        // Отрисовка остановок
        void RenderStops(svg::Document& map, const Stops& active_sorted_stops, 
            const geo::SphereProjector sphere_projector) const;

        // Отрисовка названий остановок
        void RenderStopsLabels(svg::Document& map, const Stops& active_sorted_stops, 
            const geo::SphereProjector sphere_projector) const;
    };

} // end of namespace renderer