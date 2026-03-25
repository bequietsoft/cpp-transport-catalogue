#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"


int main() {
    using namespace std;
    using namespace transport_catalogue;
    
    TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        input::InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line); // cin
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        stat::ParseAndPrintStat(catalogue, line, cout);
    }

    return 0;
}