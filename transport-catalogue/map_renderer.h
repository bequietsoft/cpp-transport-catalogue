#pragma once

#include "domain.h"
#include "geo.h"
#include "json.h"
#include "svg.h"

#include <vector>

namespace render {

    struct Offset {
        double x;    
        double y;    
    };

    struct RenderSettings {
        RenderSettings() = default;

        double width;                            // ширина изображения в пикселях
        double height;                           // высота изображения в пикселях
        double padding;                          //  отступ краёв карты от границ SVG-документа

        double line_width;                       // толщина линий, которыми рисуются автобусные маршруты
        double stop_radius;                      // радиус окружностей, которыми обозначаются остановки

        int bus_label_font_size;                 // размер текста названий автобусных маршрутов
        Offset bus_label_offset;                 // смещение надписи с названием маршрута относительно координат конечной остановки на карте

        int stop_label_font_size;                // размер текста названий остановок
        Offset stop_label_offset;                // смещение названия остановки относительно её координат на карте

        svg::Color underlayer_color;             // цвет подложки под названиями остановок и маршрутов
        double underlayer_width;                 // толщина подложки под названиями остановок и маршрутов

        std::vector<svg::Color> color_palette;   // цветовая палитра (непустой массив)
    };

    class MapRenderer {
    public:
        // Зарузка параметров отрисовки из в JSON
        void SetRenderSettings(json::Document& request_data);

        // Выгрузка параметров отрисовки в JSON
        json::Document RenderSettingsToJSON() const;

        // Доступ к параметрам отрисовки
        const RenderSettings& GetRenderSettings() const;

    private:
        RenderSettings render_settings_ {};

        // Конвертировать узел JSON в svg::Color
        svg::Color NodeToColor(const json::Node& node) const;

        // Конвертировать svg::Color в json::Node
        json::Node ColorToNode(const svg::Color& color) const;

    };

} // end of namespace renderer