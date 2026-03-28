#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

// std::string test_str = R"raw(
// 13
// Stop Tolstopaltsevo: 55.611087, 37.20829
// Stop Marushkino: 55.595884, 37.209755
// Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
// Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka
// Stop Rasskazovka: 55.632761, 37.333324
// Stop Biryulyovo Zapadnoye: 55.574371, 37.6517
// Stop Biryusinka: 55.581065, 37.64839
// Stop Universam: 55.587655, 37.645687
// Stop Biryulyovo Tovarnaya: 55.592028, 37.653656
// Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164
// Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye
// Stop Rossoshanskaya ulitsa: 55.595579, 37.605757
// Stop Prazhskaya: 55.611678, 37.603831
// 6
// Bus 256
// Bus 750
// Bus 751
// Stop Samara
// Stop Prazhskaya
// Stop Biryulyovo Zapadnoye
// )raw";

int main() {
    using namespace transport_catalogue;
    TransportCatalogue catalogue;

    //std::stringstream sin(test_str);
    {
        input::InputReader reader;
        reader.ProcessCommands(
            catalogue, 
            std::cin
            //sin
        );
    }
    stat::ProcessRequests(
        catalogue, 
        std::cin,
        //sin, 
        std::cout
    );

    return 0;
}