#pragma once

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <cmath>

namespace json {

    // Типы контейнеров JSON
    class Node; // Предварительное объявление для Dict
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Ошибка парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& output;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                output.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return {output, indent_step, indent_step + indent};
        }
    };

    // Узел документа JSON
    class Node {
    public:
        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

        // Конструкторы
        Node() : value_(nullptr) {}
        Node(std::nullptr_t) : value_(nullptr) {}
        Node(int value) : value_(value) {}
        Node(double value) : value_(value) {}
        Node(std::string value) : value_(std::move(value)) {}
        Node(bool value) : value_(value) {}
        Node(Array array) : value_(std::move(array)) {}
        Node(Dict map) : value_(std::move(map)) {}

        // Методы доступа с проверкой типа
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        // Методы проверки типа
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsNull() const;

        // Операторы сравнения
        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

    private:
        Value value_;

        // Доступ к значению value_ только для PrintNode
        friend void PrintNode(const Node& node, const PrintContext& context);
    };

    // Документ JSON
    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;
        
        bool operator==(const Document& other) const;

    private:
        Node root_;
    };

    // Загрузка документа из потока
    Document Load(std::istream& input);

    // Печать документа в поток
    void Print(const Document& doc, std::ostream& output);

}  // end of namespace json