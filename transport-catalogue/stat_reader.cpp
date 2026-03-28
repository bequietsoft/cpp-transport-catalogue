#include "stat_reader.h"

namespace transport_catalogue::stat {

// Читает данные из потока, парсит и выводит в поток результаты запросов
void ProcessRequests(const TransportCatalogue& catalogue,
    std::istream& input, std::ostream& output) {

    int stat_request_count;
    input >> stat_request_count >> std::ws;

    std::string line;
    for (int i = 0; i < stat_request_count; ++i) {
        getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}

// Парсинг данных и вывод в поток ответов на запросы
void ParseAndPrintStat(const TransportCatalogue& catalogue, 
    std::string_view request, std::ostream& output) {
    
    // Парсинг запроса формата: "Cmd X":
    auto space_pos = request.find_first_of(' ');
    std::string_view cmd = request.substr(0, space_pos);
    std::string_view id  = request.substr(space_pos + 1);

    // Запрос информации о маршруте и вывод в консоль 
    // в формате: "Bus X: R stops on route, U unique stops, L route length":
    if (cmd == "Bus") {
        output << "Bus " << id << ": ";
        auto bus_info_opt = catalogue.GetBusInfo(id);
        if (bus_info_opt.has_value()) {
            transport_catalogue::BusInfo bus_info = *bus_info_opt;
            output 
                << std::setprecision(6)
                << bus_info.stops_count << " stops on route, " 
                << bus_info.unique_stops_count << " unique stops, " 
                << bus_info.length << " route length" 
                << std::endl;
        } else {
            output 
                << "not found" << std::endl;
        }
        return;
    }

    // Запрос информации об остановке
    // в формате: "Stop X: buses bus1 bus2 ... busN"
    if (cmd == "Stop") {
        output << "Stop " << id << ": ";
        auto stop_info_opt = catalogue.GetStopInfo(id);
        if (stop_info_opt.has_value()) {
            const transport_catalogue::StopInfo stop_info = *stop_info_opt;
            const transport_catalogue::StopBuses stop_buses = stop_info.stop_buses;
            if (stop_buses.size() > 0) {
                output << "buses";
                for (auto& bus : stop_buses) {
                    output << ' ' << bus;
                }
                output << std::endl;
            } else {
                output << "no buses" << std::endl;
            }
        } else {
            output << "not found" << std::endl;
        }
        return;
    }
}

} // end of transport_catalogue::stat