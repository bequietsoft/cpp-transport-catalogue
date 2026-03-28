#pragma once

#include <iosfwd>
#include <iomanip>
#include <string_view>
#include <string>

#include "transport_catalogue.h"

namespace transport_catalogue::stat {

// Читает данные из потока, парсит и выводит в поток результаты запросов
void ProcessRequests(const TransportCatalogue& catalogue,
    std::istream& input, std::ostream& output);

// Парсинг и вывод в поток
void ParseAndPrintStat(
    const TransportCatalogue& transport_catalogue, 
    std::string_view request, std::ostream& output
);

} // end of namespace transport_catalogue::stat