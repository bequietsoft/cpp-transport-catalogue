#include "stat_reader.h"

namespace transport_catalogue::stat {
// #include <iostream>
// Читает данные из потока, парсит и выводит в поток результаты запросов
void ProcessRequests(const TransportCatalogue& catalogue,
    std::istream& input, std::ostream& output) {

    int stat_request_count;
    input >> stat_request_count >> std::ws;

    std::string line;
    for (int i = 0; i < stat_request_count; ++i) {
        std::getline(input, line);
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

        const Bus* bus_ptr = catalogue.GetBus(id);
        if (!bus_ptr) {
            output << "not found" << std::endl;
            return;
        }

        const BusInfo info = catalogue.GetBusInfo(bus_ptr);
        output 
            << std::setprecision(6)
            << info.total_stops << " stops on route, " 
            << info.unique_stops << " unique stops, " 
            << info.length_km << " route length, "
            << info.curvature << " curvature"
            << std::endl;
        return;
    }
    
    // Запрос информации об остановке
    // в формате: "Stop X: buses bus1 bus2 ... busN"
    if (cmd == "Stop") {
        output << "Stop " << id << ": ";

        const Stop* stop_ptr = catalogue.GetStop(id);
        if (!stop_ptr) {
            output << "not found" << std::endl;
            return;
        }

        const StopBuses* stop_buses_ptr = catalogue.GetStopBuses(stop_ptr);
        if (stop_buses_ptr->size() > 0) {
            output << "buses";
            for (auto bus : *stop_buses_ptr) {
                output << ' ' << bus;
            }
            output << std::endl;
        } else {
            output << "no buses" << std::endl;
        }

        return;
    }
}

} // end of transport_catalogue::stat