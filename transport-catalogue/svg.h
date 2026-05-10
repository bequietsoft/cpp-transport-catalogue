#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <optional>
#include <variant>

namespace svg {

    // Параметры свойтв стиля
    struct Rgb {
        uint8_t red {0};
        uint8_t green {0};
        uint8_t blue {0};
    };

    struct Rgba {
        uint8_t red {0};
        uint8_t green {0};
        uint8_t blue {0};
        double opacity {1.0};
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        ROUND,
        MITER,
        MITER_CLIP,
    };
    
    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor {"none"};

    // Операторы вывода свойств стиля в поток
    std::ostream& operator<< (std::ostream& os, Color color);
    std::ostream& operator<< (std::ostream& os, StrokeLineCap slc);
    std::ostream& operator<< (std::ostream& os, StrokeLineJoin slj);

    // Базовый класс свойств стиля
    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }

            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }

            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }

            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }

            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }

        bool HasAttrs() const {
            return fill_color_ || stroke_color_ || stroke_width_ || stroke_line_cap_ || stroke_line_join_;
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&, если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    // Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
    // Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
    struct RenderContext {
        RenderContext(std::ostream& out) 
        : out(out) {}

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent) {}

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) out.put(' ');
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    // Абстрактный базовый класс Object служит для унифицированного хранения
    // конкретных тегов SVG-документа
    // Реализует паттерн "Шаблонный метод" для вывода содержимого тега
    class Object {
    public:
        virtual ~Object() = default;
        void Render(const RenderContext& context) const;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    // Точка на плоскости
    struct Point {
        Point() = default;
        Point(double x, double y)
        : x(x), y(y) {}

        double x = 0;
        double y = 0;
    };

    // Класс Circle моделирует элемент <circle> для отображения круга
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle(Point center = {0., 0.}, double radius = 1.) 
        : center_(center), radius_(radius) {}

        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        virtual void RenderObject(const RenderContext& context) const override;

        Point center_ {0., 0.};
        double radius_ {1.};
    };

    // Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline() = default;

        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);
        
        // // Вовращает количество точек
        // size_t GetSize() const { return points_.size(); }
        // // Вовращает точки
        // std::deque<Point>& GetPoints() { return points_; }

    private:
        virtual void RenderObject(const RenderContext& context) const override;

        std::deque<Point> points_;
    };

    // Класс Text моделирует элемент <text> для отображения текста
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
    class Text final : public Object, public PathProps<Text> {
    public:
        Text(Point pos = {0., 0.}, Point offset = {0., 0.}, uint32_t font_size = 1)
        : pos_(pos), offset_(offset), font_size_(font_size) {}

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void EscapeDataOut(const std::string& data, std::ostream& out) const;

        virtual void RenderObject(const RenderContext& context) const override;

        Point pos_ {0., 0.};
        Point offset_ {0., 0.};
        uint32_t font_size_ {1};
        std::string font_family_;
        std::string font_weight_;

        std::string data_ {};
    };

    // Абстрактный класс контейнер
    class ObjectContainer {
    public:
        template <typename T>
        ObjectContainer& Add(T obj) {
            static_assert(std::is_base_of_v<Object, T>, "Template parameter T must be a class derived from svg::Object");
            AddPtr(std::make_unique<T>(obj));
            return *this;
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    // Документ (SVG-контейнер)
    class Document final : public ObjectContainer {
    public:
        Document() = default;

        // Реализует добавление в контейнер объекта-наследника от svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) final override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::deque<std::unique_ptr<Object>> objects_;
    };

    // Интерфейс для отображаемых объектов
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

} // end of namespace svg