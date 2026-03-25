#pragma once

#include <iosfwd>
#include <iomanip>
#include <string_view>
#include <string>

#include "transport_catalogue.h"

namespace transport_catalogue::stat {

void ParseAndPrintStat(
    const TransportCatalogue& transport_catalogue, 
    std::string_view request, std::ostream& output
);

} // end of namespace transport_catalogue::stat