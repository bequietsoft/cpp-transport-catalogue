#pragma once

#include "transport_catalogue.h"

#include <iosfwd>
#include <iomanip>
#include <string_view>
#include <string>

namespace transport_catalogue::stat {

using Bus = transport_catalogue::Bus;
using Stop = transport_catalogue::Stop;
using StopBuses = transport_catalogue::StopBuses;

// Читает данные из потока, парсит и выводит в поток результаты запросов
void ProcessRequests(const TransportCatalogue& catalogue,
    std::istream& input, std::ostream& output);

// Парсинг и вывод в поток
void ParseAndPrintStat(
    const TransportCatalogue& transport_catalogue, 
    std::string_view request, std::ostream& output
);

} // end of namespace transport_catalogue::stat