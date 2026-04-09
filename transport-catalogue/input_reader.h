#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <istream>
#include <string_view>
#include <vector>

namespace transport_catalogue::input {

struct CommandDescription {    
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator! () const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

struct Distance {
    std::string_view to_stop; // Остановка, до которой задано рассотяние
    double length_m;          // Рассотяние в метрах
};

class InputReader {
public:
    // Читает данные из потока, парсит и применяет к справочнику
    void ProcessCommands(TransportCatalogue& catalogue, std::istream& is);

    // Парсит данных в структуру CommandDescription и сохраняет результат в commands_
    void ParseLine(std::string_view line);

    // Наполняет данными транспортный справочник, используя команды из commands_
    void ApplyCommands(TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> commands_;
    std::vector<Distance> distances_;
};

} // end of namespace transport_catalogue::input