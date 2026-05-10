#include "svg.h"

namespace svg {

    using namespace std::literals;

    // Операторы вывода свойств стиля в поток

    std::ostream& operator<< (std::ostream& os, Color color) {    
        std::visit([&os](const auto& c) {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                os << "none";
            } 
            else if constexpr (std::is_same_v<T, std::string>) {
                os << c;
            } 
            else if constexpr (std::is_same_v<T, Rgb>) {
                os << "rgb("
                << static_cast<int>(c.red) << ","
                << static_cast<int>(c.green) << ","
                << static_cast<int>(c.blue) << ")";
            } 
            else if constexpr (std::is_same_v<T, Rgba>) {
                os << "rgba("
                << static_cast<int>(c.red) << ","
                << static_cast<int>(c.green) << ","
                << static_cast<int>(c.blue) << ","
                << c.opacity << ")";
            }

        }, color);
        return os;
    }

    std::ostream& operator<< (std::ostream& os, StrokeLineCap slc) {
        switch (slc) {
            case StrokeLineCap::BUTT:   os << "butt";   break;
            case StrokeLineCap::ROUND:  os << "round";  break;
            case StrokeLineCap::SQUARE: os << "square"; break;
            default: os << "unknown_stroke_linecap"; break;
        };
        return os;
    }

    std::ostream& operator<< (std::ostream& os, StrokeLineJoin slj) {
        switch (slj) {
            case StrokeLineJoin::ARCS:  os << "arcs";   break;
            case StrokeLineJoin::BEVEL: os << "bevel";   break;
            case StrokeLineJoin::ROUND: os << "round";   break;
            case StrokeLineJoin::MITER: os << "miter";   break;
            case StrokeLineJoin::MITER_CLIP: os << "miter-clip";   break;
            default: os << "unknown_stroke_linejoin"; break;
        };
        return os;
    }

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context); // Делегируем вывод тега своим подклассам
        context.out << '\n';
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out 
            << "<circle "sv
            << "cx=\""sv << center_.x << "\" "sv
            << "cy=\""sv << center_.y << "\" "sv
            << "r=\""sv  << radius_   << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- PolyLine ----------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ',' << points_[i].y;
            if (i < points_.size() - 1) out << ' ';
        }
        out << "\""sv;
        if (HasAttrs()) RenderAttrs(out); else out << " ";
        out << "/>"sv;
    }

    // ---------- Text --------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t font_size) {
        font_size_ = font_size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::EscapeDataOut(const std::string& data, std::ostream& out) const {
        for (char c : data) {
            switch (c) {
                case '\"': out << "&quot;"; break;
                case '\'': out << "&apos;"; break;
                case '<':  out << "&lt;";   break;
                case '>':  out << "&gt;";   break;
                case '&':  out << "&amp;";  break;
                default:   out.put(c);        break;
            }
        }
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << font_size_ << "\""sv;
        if (font_family_.size() > 0) out << " font-family=\""sv << font_family_ << "\""sv;
        if (font_weight_.size() > 0) out << " font-weight=\""sv << font_weight_ << "\""sv;
        out << ">"sv;
        if (data_.size() > 0) EscapeDataOut(data_, out);
        out << "</text>"sv;
    }

    // ---------- Document ----------------

    // Реализует добавление в документ объекта-наследника от svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        RenderContext context(out, 2, 2);
        for (const auto& obj : objects_) { 
            obj->Render(context);
        }
        out << "</svg>"sv;
    }

} // end of namespave svg