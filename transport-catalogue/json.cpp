#include "json.h"

namespace json {

    // Утилиты парсера
    namespace json::detail {

        // Пропускает пробелы, табуляции и символы перевода строки
        void DropSpaces(std::istream& input) {
            while (input.peek() != EOF && std::isspace(input.peek())) {
                input.get();
            }
        }

        // Загрузка литерала формата по образцу expected
        Node LoadLiteral(std::istream& input, const std::string& expected, Node value) {
            std::string actual;
            actual.reserve(expected.size());

            for (size_t i = 0; i < expected.size(); ++i) {
                char c;
                if (!input.get(c)) 
                    throw ParsingError("Unexpected end of input");

                actual += c;
            }

            if (actual != expected) 
                throw ParsingError("Invalid literal: expected " + expected);

            return value;
        }

    } // end of namespace json::detail 

    // Парсинг узлов JSON
    namespace json::parsing {

        Node LoadNode(std::istream& input); // Предварительное объявление
        
        Node LoadNumber(std::istream& input) {
            std::string num_str;
            char c;

            // Считываем все символы числа
            while (std::isdigit(input.peek()) ||
                input.peek() == '.' ||
                input.peek() == 'e' ||
                input.peek() == 'E' ||
                input.peek() == '-' ||
                input.peek() == '+') {

                input.get(c);
                num_str += c;
            }

            if (num_str.find('.') != std::string::npos ||
                num_str.find('e') != std::string::npos ||
                num_str.find('E') != std::string::npos) {
                // Если точка или экспонента - double
                try {
                    double value = std::stod(num_str);
                    return Node(value);
                } catch (...) {
                    throw ParsingError("Invalid double value");
                }
            } 
            else {
                // Иначе - int
                try {
                    int value = std::stoi(num_str);
                    return Node(value);
                } catch (...) {
                    throw ParsingError("Invalid int value");
                }
            }
        }

        Node LoadString(std::istream& input) {
            std::string result;
            char c;

            if (input.get() != '"') 
                throw ParsingError("Wrong string begining");

            while (input.get(c)) {
                if (c == '"') break;
                if (c == '\\') {
                    if (!input.get(c)) 
                        throw ParsingError("Unterminated escape sequence");
                    
                    switch (c) {
                        case 'r': result += '\r'; break;
                        case 'n': result += '\n'; break;
                        case 't': result += '\t'; break;
                        case 'b': result += '\b'; break;
                        case '"': result += '\"'; break;
                        case '\\': result += '\\'; break;
                        default: 
                            throw ParsingError("Unknown escape sequence");
                    }
                } else {
                    result += c;
                }
            }
            
            if (!input) 
                throw ParsingError("Unterminated string");

            return Node(std::move(result));
        }

        Node LoadArray(std::istream& input) {
            Array result;

            if (input.get() != '[') 
                throw ParsingError("Wrong array begining");

            for (char c; input >> c && c != ']';) {
                if (c != ',') input.putback(c);
                result.push_back(LoadNode(input));
            }

            if (input.fail()) 
                throw ParsingError("Invalid array format");

            return Node(std::move(result));
        }

        Node LoadDict(std::istream& input) {
            Dict result;

            if (input.get() != '{') 
                throw ParsingError("Wrong dictionary begining");

            for (char c; input >> c && c != '}';) {
                if (c != ',') input.putback(c);

                json::detail::DropSpaces(input);
                if (input.peek() != '"') 
                    throw ParsingError("Expected string key");

                std::string key = LoadString(input).AsString();

                json::detail::DropSpaces(input);
                if (input.get() != ':') 
                    throw ParsingError("Expected colon after key");
                
                json::detail::DropSpaces(input);
                result[std::move(key)] = LoadNode(input);
            }

            if (input.fail()) 
                throw ParsingError("Invalid dictionary format");

            return Node(std::move(result));
        }

        Node LoadNode(std::istream& input) {
            json::detail::DropSpaces(input);
            int c = input.peek();

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 'n') {
                return json::detail::LoadLiteral(input, "null", Node(nullptr));
            } else if (c == 't') {
                return json::detail::LoadLiteral(input, "true", Node(true));
            } else if (c == 'f') {
                return json::detail::LoadLiteral(input, "false", Node(false));
            } else if (c == '-' || std::isdigit(c)) {
                return LoadNumber(input);
            } else {
                throw ParsingError("Unexpected character: " + std::string(1, c));
            }
        
        }

    }  // end of namespace json::parsing 


    // Узел документа JSON

    // Методы доступа с проверкой типа
    int Node::AsInt() const {
        if (!IsInt()) 
            throw std::logic_error("Node is not an int");
        return std::get<int>(value_);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(std::get<int>(value_));
        } 
        else 
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        throw std::logic_error("Node cannot be converted to double");
    }

    const std::string& Node::AsString() const {
        if (!IsString()) 
            throw std::logic_error("Node is not a string");
        return std::get<std::string>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool())
            throw std::logic_error("Node is not a bool");
        return std::get<bool>(value_);
    }
    
    const Array& Node::AsArray() const {
        if (!IsArray())
            throw std::logic_error("Node is not an Array");
        return std::get<Array>(value_);
    }

    const Dict& Node::AsDict() const {
        if (!IsDict())
            throw std::logic_error("Node is not a Dict");
        return std::get<Dict>(value_);
    }

    // Методы проверки типа
    bool Node::IsInt() const { return std::holds_alternative<int>(value_); }
    bool Node::IsDouble() const { return IsInt() || IsPureDouble(); }
    bool Node::IsPureDouble() const { return std::holds_alternative<double>(value_); }
    bool Node::IsString() const { return std::holds_alternative<std::string>(value_); }
    bool Node::IsBool() const { return std::holds_alternative<bool>(value_); }
    bool Node::IsArray() const { return std::holds_alternative<Array>(value_); }
    bool Node::IsDict() const { return std::holds_alternative<Dict>(value_); }
    bool Node::IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }

    // Операторы сравнения
    bool Node::operator==(const Node& other) const {
        return value_ == other.value_;
    }

    bool Node::operator!=(const Node& other) const {
        return !(*this == other);
    }


    // Документ JSON

    // Загрузка документа из потока
    Document Load(std::istream& input) {
        auto doc = Document {json::parsing::LoadNode(input)}; // move temp object prevents copy ellision

        // После парсинга проверяем, что остались только пробелы
        json::detail::DropSpaces(input);
        if (input.peek() != EOF) 
            throw ParsingError("Extra data after JSON");
        
        return doc;
    }


    // Внешние функции для печати документа и узлов в поток

    // Шаблон функции печати
    template <typename T>
    void PrintValue(const T& value, const PrintContext& context) {
        context.output << value;
    }

    // Специализация для double
    // * имитация Linux-рендера double для точного соотвествия финальным тестам 
    template <>
    void PrintValue<double>(const double& value, const PrintContext& context) {

        if (value >= 1e6) {  
            std::ostringstream oss;
            oss << std::scientific << std::setprecision(5) << value;
            std::string result = oss.str();

            // Удаляем все нули перед e+
            for(;;) {
                size_t pos_plus = result.find("0e+");
                if (pos_plus != std::string::npos) {
                    result.replace(pos_plus, 1, "");
                } else break;
            }

            context.output << result;
        } else {
            context.output 
                << std::defaultfloat 
                << std::setprecision(6) 
                << value;
        }
    }

    // Специализация для nullptr
    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& context) {
        context.output << "null";
    }

    // Специализация для представлений строк
    template <>
    void PrintValue<std::string_view>(const std::string_view& str_view, const PrintContext& context) {
        context.output << '"';
        for (char c : str_view) {
            switch (c) {
                case '\n': context.output << "\\n"; break;
                case '\r': context.output << "\\r"; break;
                case '\t': context.output << "\\t"; break;
                case '\b': context.output << "\\b"; break;
                case '\\': context.output << "\\\\"; break;
                case '"' : context.output << "\\\""; break;
                default: context.output << c;
            }
        }
        context.output << '"';
    }

    // Специализация для строк
    template <>
    void PrintValue<std::string>(const std::string& str, const PrintContext& context) {
        PrintValue(std::string_view(str), context);
    }

    // Специализация для bool
    template <>
    void PrintValue<bool>(const bool& value, const PrintContext& context) {
        context.output << (value ? "true" : "false");
    }

    // Специализация для массивов
    template <>
    void PrintValue<Array>(const Array& arr, const PrintContext& context) {
        context.output << "[\n";
        auto indented = context.Indented();

        if (arr.empty()) {
            context.PrintIndent();
            context.output << "]";
            return;
        }

        bool first = true;
        for (const auto& item : arr) {
            if (!first) {
                context.output << ",\n";
            }
            indented.PrintIndent();
            PrintNode(item, indented);
            first = false;
        }

        context.output << '\n';
        context.PrintIndent();
        context.output << ']';
    }

    // Специализация для словарей
    template <>
    void PrintValue<Dict>(const Dict& dict, const PrintContext& context) {
        
        context.output << "{\n";
        auto indented = context.Indented();

        if (dict.empty()) {
            context.PrintIndent();
            context.output << "}";
            return;
        }

        bool first = true;
        for (const auto& [key, value] : dict) {
            if (!first) {
                context.output << ",\n";
            }
            indented.PrintIndent();
            context.output << '"' << key << '"' << ": ";
            PrintNode(value, indented);
            first = false;
        }

        context.output << '\n';
        context.PrintIndent();
        context.output << '}';
    }

    // Печать узла ("посетитель")
    void PrintNode(const Node& node, const PrintContext& context) {    
        std::visit([&context](const auto& value) {
            PrintValue(value, context);
        }, node.value_);
    }

    // Печать документа в поток
    void Print(const Document& doc, std::ostream& output) {
        PrintContext context{output};
        PrintNode(doc.GetRoot(), context);
    }

}  // end of namespace json