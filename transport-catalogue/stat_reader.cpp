#include "stat_reader.h"

namespace transport_catalogue::stat {

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, 
    std::string_view request, std::ostream& output) {
    
    // Парсинг запроса формата: "Cmd X":
    auto space = request.find_first_of(' ');
    std::string cmd = std::string(request.substr(0, space));
    std::string id  = std::string(request.substr(space + 1));

    // Запрос информации о маршруте и вывод в консоль 
    // в формате: "Bus X: R stops on route, U unique stops, L route length":
    if (cmd == "Bus") {
        transport_catalogue::BusInfo info;
        output << "Bus " << id << ": ";
        if (transport_catalogue.GetBusInfo(id, &info)) {
            output 
                << std::setprecision(6)
                << info.stops_count << " stops on route, " 
                << info.unique_stops_count << " unique stops, " 
                << info.length << " route length" 
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
        transport_catalogue::StopInfo info;
        output << "Stop " << id << ": ";
        if (transport_catalogue.GetStopInfo(id, &info)) {
            if (info.stop_buses.size() > 0) {
                output << "buses";
                for (auto& bus : info.stop_buses) {
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