#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include ".tests/tests.h"

int main() {
    test_catalog(); test_files(); return 0;

    using namespace transport_catalogue;
    TransportCatalogue catalogue;
    
    {
        input::InputReader reader;
        reader.ProcessCommands(catalogue, std::cin);
    }
    stat::ProcessRequests(catalogue, std::cin, std::cout);

    return 0;
}