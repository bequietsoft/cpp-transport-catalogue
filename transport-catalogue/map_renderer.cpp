#include "map_renderer.h"

namespace render  {

    // Зарузка параметров отрисовки из в JSON
    void MapRenderer::SetRenderSettings(json::Document& request_data) {
        const auto& root = request_data.GetRoot().AsMap();

        if (root.count("render_settings")) {
            const auto& render_settings = root.at("render_settings").AsMap();

            render_settings_.width = render_settings.at("width").AsDouble();
            render_settings_.height = render_settings.at("height").AsDouble();
            render_settings_.padding = render_settings.at("padding").AsDouble();

            render_settings_.line_width = render_settings.at("line_width").AsDouble();
            render_settings_.stop_radius = render_settings.at("stop_radius").AsDouble();
            
            render_settings_.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
            const auto& bus_label_offset = render_settings.at("bus_label_offset").AsArray();
            render_settings_.bus_label_offset = { 
                bus_label_offset[0].AsDouble(), 
                bus_label_offset[1].AsDouble()
            };
            
            render_settings_.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
            const auto& stop_label_offset = render_settings.at("stop_label_offset").AsArray();
            render_settings_.stop_label_offset = { 
                stop_label_offset[0].AsDouble(), 
                stop_label_offset[1].AsDouble()
            };
            
            const auto& underlayer_color = render_settings.at("underlayer_color");
            render_settings_.underlayer_color = NodeToColor(underlayer_color); 
            render_settings_.underlayer_width = render_settings.at("underlayer_width").AsDouble();
            
            const auto& color_palette = render_settings.at("color_palette").AsArray();
            for(const auto& color_node : color_palette)
                render_settings_.color_palette.push_back(NodeToColor(color_node));                
        }
    }

    // Выгрузка параметров отрисовки в JSON
    json::Document MapRenderer::RenderSettingsToJSON() const {
        json::Dict render_settings;

        render_settings["width"] = json::Node(render_settings_.width);
        render_settings["height"] = json::Node(render_settings_.height);
        render_settings["padding"] = json::Node(render_settings_.padding);

        render_settings["line_width"] = json::Node(render_settings_.line_width);
        render_settings["stop_radius"] = json::Node(render_settings_.stop_radius);

        render_settings["bus_label_font_size"] = json::Node(render_settings_.bus_label_font_size);
        render_settings["bus_label_offset"] = json::Node(json::Array{
            json::Node(render_settings_.bus_label_offset.x),
            json::Node(render_settings_.bus_label_offset.y)
        });

        render_settings["stop_label_font_size"] = json::Node(render_settings_.stop_label_font_size);
        render_settings["stop_label_offset"] = json::Node(json::Array{
            json::Node(render_settings_.stop_label_offset.x),
            json::Node(render_settings_.stop_label_offset.y)
        });

        render_settings["underlayer_color"] = ColorToNode(render_settings_.underlayer_color);
        render_settings["underlayer_width"] = json::Node(render_settings_.underlayer_width);

        json::Array color_palette_array;
        color_palette_array.reserve(render_settings_.color_palette.size());
        for (const auto& color : render_settings_.color_palette) {
            color_palette_array.emplace_back(ColorToNode(color));
        }
        render_settings["color_palette"] = json::Node(std::move(color_palette_array));

        json::Dict response;
        response["render_settings"] = json::Node(std::move(render_settings));
        return json::Document(json::Node(std::move(response)));
    }

    // Доступ к параметрам отрисовки
    const RenderSettings& MapRenderer::GetRenderSettings() const {
        return render_settings_;
    }

    // Конвертировать узел JSON в svg::Color
    svg::Color MapRenderer::NodeToColor(const json::Node& node) const {
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

        // По умолчанию возвращаем чёрный цвет
        return svg::Color(svg::Rgb{0, 0, 0});
    }

    // Конвертировать svg::Color в json::Node
    json::Node MapRenderer::ColorToNode(const svg::Color& color) const {
        return std::visit([this](const auto& c) -> json::Node {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                return json::Node(nullptr);
            }

            if constexpr (std::is_same_v<T, std::string>) {
                return json::Node(c);
            }

            if constexpr (std::is_same_v<T, svg::Rgb>) {
                return json::Node(json::Array{
                    json::Node(static_cast<int>(c.red)),
                    json::Node(static_cast<int>(c.green)),
                    json::Node(static_cast<int>(c.blue))
                });
            }

            if constexpr (std::is_same_v<T, svg::Rgba>) {
                return json::Node(json::Array{
                    json::Node(static_cast<int>(c.red)),
                    json::Node(static_cast<int>(c.green)),
                    json::Node(static_cast<int>(c.blue)),
                    json::Node(c.opacity)
                });
            }

            // По умолчанию возвращаем null
            return json::Node(nullptr);
        }, color);
    }

} // end of namespace renderer