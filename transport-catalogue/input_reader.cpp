#include "input_reader.h"

namespace transport_catalogue::input {

// Удаляет пробелы в начале и конце строки
std::string_view Trim(std::string_view str) {
    const auto start = str.find_first_not_of(' ');
    if (start == str.npos) return {};
    return str.substr(start, str.find_last_not_of(' ') + 1 - start);
}

// Разбивает строку string на n строк, с помощью строки-разделителя spl
std::vector<std::string_view> Split(std::string_view str, std::string_view spl) {
    assert(!str.empty());
    assert(!spl.empty());

    std::vector<std::string_view> result;
    size_t pos = 0;
    while (pos < str.size()) {
        auto spl_pos = str.find(spl, pos);
        if (spl_pos == str.npos) {
            spl_pos = str.size();
        }
        if (auto substr = Trim(str.substr(pos, spl_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = spl_pos + spl.size();
    }
    return result;
}

// Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
// Широта задаётся в градусах от -90.0 (южный полюс) до +90.0 (северный полюс)
// Долгота задаётся в градусах от -180.0 (запад от Гринвича) до +180.0 (восток от Гринвича)
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");
    
    auto comma_split = Split(str, ",");
    if (comma_split.size() < 2) return {nan, nan};

    return {
        std::stod(std::string(comma_split[0])),
        std::stod(std::string(comma_split[1]))
    };
}

// Парсит расстояния от остановки до соседних остановок
// Возвращает массив расстояний до остановок
std::vector<Distance> ParseDistances(std::string_view str) {
    auto comma_split = Split(str, ",");
    if (comma_split.size() < 3) return {};

    std::vector<Distance> result;
    result.reserve(comma_split.size() - 2);
    for (size_t i = 2; i < comma_split.size(); ++i) {
        auto to_split = Split(comma_split[i], "m to ");
        assert(to_split.size() == 2);
        double distance = std::stod(std::string(to_split[0]));
        assert(distance > 0.0 && distance <= 1000000.0);
        result.push_back(Distance{ to_split[1], distance });
    }
    return result;
}

// Парсит маршрут.
// Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
// Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
std::vector<std::string_view> ParseRoute(std::string_view raw_route) {
    
    // Кольцевой маршрут (разделитель '>'):
    if (raw_route.find('>') != raw_route.npos) {
        return Split(raw_route, ">");
    }

    // Челночный маршрут (разделитель '-'):
    auto stops = Split(raw_route, "-");
    std::vector<std::string_view> route;
    route.reserve(2 * stops.size() - 1);

    // Прямой проход
    route.insert(route.end(), stops.begin(), stops.end());

    // Обратный проход (без последней остановки)
    route.insert(route.end(), stops.rbegin() + 1, stops.rend());

    return route;
}

// Парсинг строки запроса в формат CommandDescription
CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) return {};

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) return {};

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) return {};

    return {
        std::string(std::string_view(line).substr(0, space_pos)),
        std::string(std::string_view(line).substr(not_space, colon_pos - not_space)),
        std::string(std::string_view(line).substr(colon_pos + 1))
    };
}

// Читает данные из потока, парсит и применяет к справочнику
void InputReader::ProcessCommands(TransportCatalogue& catalogue, std::istream& is) {
    size_t commands_count;
    is >> commands_count >> std::ws;

    std::string line;
    for (size_t i = 0; i < commands_count; ++i) {
        getline(is, line);
        ParseLine(line);
    }

    ApplyCommands(catalogue);
}

// Парсит строку в структуру CommandDescription и сохраняет результат в commands_
void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

// Выполняет запросы к справочнику в нужной последовательности
void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    
    // Обработка команд создания остановок
    for (const auto& cmd : commands_) {
        if (cmd.command == "Stop") {
            geo::Coordinates location = ParseCoordinates(cmd.description);
            catalogue.AddStop(cmd.id, location);
        }
    }

    // Обработка команд привязки расстояний
    for (const auto& cmd : commands_) {
        if (cmd.command == "Stop") {
            std::vector<Distance> distances = ParseDistances(cmd.description);
            for (auto dist: distances) {            
                catalogue.AddDistance(cmd.id, dist.to_stop, dist.length_m);
            }
        }
    }

    // Обработка команд создания маршрутов
    for (const auto& cmd : commands_) {
        if (cmd.command == "Bus") {
            std::vector<std::string_view> stops = ParseRoute(cmd.description);
            catalogue.AddBus(cmd.id, stops);
        }
    }
}

} // end of namespace transport_catalogue::input
