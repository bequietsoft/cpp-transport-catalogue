#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>

#include "../transport_catalogue.h"
#include "../input_reader.h"
#include "../stat_reader.h"

#include "./log_duration.h"


std::string ReadStringFromFile(const std::string& file_name) {
    std::ifstream fin(file_name);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file: " + file_name);
    }

    std::string result;
    fin.seekg(0, std::ios::end);
    result.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);

    result.assign(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
    return result;
}

std::string PrintChar(char ch) {
    switch (ch) {
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\b': return "\\b";
        case '\r': return "\\r";
        case '\\': return "\\\\";  // Обратный слеш
        case '"':  return "\\\""; // Кавычка
        case '\'': return "\\'";  // Апостроф
        default:   return std::string(1, ch);
    }
}

// ANSI escape коды для цветов
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string GRAY = "\033[90m";
const std::string RESET = "\033[0m";

bool CompareStrings(const std::string& str1, const std::string& str2, const size_t limit = 0) {
    
    size_t len1 = str1.size();
    size_t len2 = str2.size();
    size_t dif_len = (len1 > len2) ? (len1 - len2) : (len2 - len1);
    size_t max_len = std::max(len1, len2);
    size_t d_count = 0;
    size_t cur_line1 = 1;
    size_t cur_line2 = 1;

    std::cout << GRAY << "Результат (" << len1 << " байт)" << RESET;
    if (len1 < 256) std::cout << ":\n" << str1;
    std::cout << "\n";
    std::cout << GRAY << "Эталон (" << len2 << " байт)" << RESET;
    if (len2 < 256) std::cout << ":\n" << str2;
    std::cout << "\n";

    if (dif_len >= limit) {
        std::cout 
            << YELLOW << "разница в размере " << dif_len << " байт" 
            << RESET << "\n"; 
        return false;
    }

    std::cout 
        << GRAY << "Сравнение (позиции 0-" << max_len - 1 << ")"
        << RESET << "\n";

    for (size_t i = 0; i < max_len; ++i) {

        if (i >= len1 || i >= len2 || str1[i] != str2[i]) {
            ++d_count;
            std::cout 
                << GRAY 
                << std::setw(4) << std::setfill('0') << cur_line1 << "-" 
                << std::setw(4) << std::setfill('0') << cur_line2 << " [" 
                << std::setw(4) << std::setfill('0') << i << "]\t";
        }

        if (i >= len1) {
            std::cout 
                << RED << "+ '" << PrintChar(str2[i]) << "'" << " (эталон)" 
                << RESET << "\n";
        } 
        else 
        if (i >= len2) {
            std::cout 
                << RED << "- '" << PrintChar(str1[i]) << "'" << " (результат)" 
                << RESET << "\n";
        } 
        else 
        if (str1[i] != str2[i]) {
            std::cout 
                << GRAY 
                << YELLOW << "* '" 
                << PrintChar(str1[i]) << "' → '" << PrintChar(str2[i]) << "'" << " (изменён)" 
                << RESET << "\n";
        }

        if (limit > 0 && d_count > limit) {
            std::cout 
                << YELLOW << "найдено " << limit << " различий" 
                << RESET << "\n"; 
            return false;
        }

        if (str1[i] == '\n') ++cur_line1;
        if (str2[i] == '\n') ++cur_line2;
    } 
    
    if (d_count == 0) std::cout << GREEN << "идентичны\n";
    std::cout << RESET << "\n" ;
    return true;
}

void test_catalog() {
    using namespace transport_catalogue;
    TransportCatalogue catalogue;

    std::string stop1 = "stop1"; 
    std::string_view stop2 = "stop2";
    catalogue.AddStop(std::string_view(stop1), geo::Coordinates(100.00, 100.00));
    catalogue.AddStop(stop2, geo::Coordinates(110.00, 100.00));
    catalogue.AddStop(std::string_view("stop3"), geo::Coordinates(110.00, 110.00));
    catalogue.AddStop(std::string_view("stop4"), geo::Coordinates(110.00, 100.00));
    assert(catalogue.GetStop(stop1) != nullptr);
    assert(catalogue.GetStop(std::string_view("stop2")) != nullptr);
    assert(catalogue.GetStop(std::string_view("stop3")) != nullptr);
    
    std::vector<std::string_view> route = { 
        std::string_view("stop1"), 
        std::string_view("stop2"), 
        std::string_view("stop3"),
        std::string_view("stop4"),
        std::string_view("stop1")
    };
    catalogue.AddBus(std::string_view("bus1"), route);

    Bus* bus_ptr = catalogue.GetBus(std::string_view("bus1"));

    assert(catalogue.GetBus(bus_ptr->id) != nullptr);
    assert(catalogue.GetBus(std::string_view("bus1")) != nullptr);
    assert(catalogue.GetBus("bus2") == nullptr);

    assert(bus_ptr->route.size() == 5);
    assert(catalogue.GetRouteUniqueStopsCount(bus_ptr->route) == 4);
    double len = catalogue.GetRouteLength(bus_ptr->route);
    assert(std::fabs(len - 2983664.13564) < 0.00001);

    std::cout << GREEN << "все тесты пройдены" << RESET << "\n";
}

bool test_file(const std::string& in_file_name, const std::string& out_file_name) {
    using namespace transport_catalogue;

    std::string str_in = ReadStringFromFile(in_file_name);
    std::string str_out;

    {
        LogDuration logduration("TransportCatalogue (" + in_file_name + ")");
        TransportCatalogue catalogue;
        
        std::stringstream sin(str_in);
        std::stringstream sout;

        {
            LogDuration logduration("TransportCatalogue::input");
            input::InputReader reader;
            reader.ProcessCommands(catalogue, sin);
        }

        {
            LogDuration logduration("TransportCatalogue::stat");
            stat::ProcessRequests(catalogue, sin, sout);
            str_out = sout.str();
        }
    }

    std::string str_out_etalon = ReadStringFromFile(out_file_name);
    return CompareStrings(str_out, str_out_etalon, 10);
}

void test_files() {
    bool a = test_file(".tests/tsB_case0_input.txt", ".tests/tsB_case0_output.txt");
    bool b = test_file(".tests/tsB_case1_input.txt", ".tests/tsB_case1_output.txt");
    bool c = test_file(".tests/tsB_case2_input.txt", ".tests/tsB_case2_output.txt");
    assert(a && b && c);
}