#pragma once

#include "../transport_catalogue.h"
#include "../input_reader.h"
#include "../stat_reader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, y) LogDuration UNIQUE_VAR_NAME_PROFILE(x, y)

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string GRAY = "\033[90m";
const std::string WHITE = "\033[97m";
const std::string RESET = "\033[0m";
const size_t MAX_OUTPUT_LENGTH = 300;

class LogDuration {
public:
    using Clock = std::chrono::steady_clock;

    LogDuration(const std::string& id, std::ostream& dst_stream = std::cerr)
        : id_(id)
        , dst_stream_(dst_stream) {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;
        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        double duration_ms = duration_cast<microseconds>(dur).count() / 1000.0;
        dst_stream_
            << WHITE
            << id_ << ": "
            << std::fixed << std::setprecision(2) << duration_ms << " ms"
            << RESET << '\n';
    }

private:
    const std::string id_;
    const Clock::time_point start_time_ = Clock::now();
    std::ostream& dst_stream_;
};

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
        case '\\': return "\\\\";
        case '"':  return "\\\""; 
        case '\'': return "\\'"; 
        default:   return std::string(1, ch);
    }
}

bool CompareStrings(const std::string& str1, const std::string& str2, const size_t limit = 0) {
    
    size_t len1 = str1.size();
    size_t len2 = str2.size();
    size_t dif_len = (len1 > len2) ? (len1 - len2) : (len2 - len1);
    size_t max_len = std::max(len1, len2);
    size_t d_count = 0;
    size_t cur_line1 = 1;
    size_t cur_line2 = 1;

    std::cout << GRAY << "Результат (" << len1 << " байт)" << RESET;
    if (len1 < MAX_OUTPUT_LENGTH) std::cout << ":\n" << str1;
    std::cout << "\n";
    std::cout << GRAY << "Эталон (" << len2 << " байт)" << RESET;
    if (len2 < MAX_OUTPUT_LENGTH) std::cout << ":\n" << str2;
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

void TestCatalog() {
    using namespace transport_catalogue;
    
    {
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

        const Bus* bus_ptr = catalogue.GetBus(std::string_view("bus1"));

        assert(catalogue.GetBus(bus_ptr->id) != nullptr);
        assert(catalogue.GetBus(std::string_view("bus1")) != nullptr);
        assert(catalogue.GetBus("bus2") == nullptr);

        const Stop* stop = catalogue.GetStop(std::string_view("stop2"));
        const StopBuses* buses = catalogue.GetStopBuses(stop);
        assert(buses != nullptr);
        assert(buses->size() == 1);
        assert(buses->contains("bus1"));
        assert(buses->find("bus1") != buses->end());
    }

    {
        TransportCatalogue catalogue;

        // Добавляем остановки с координатами
        std::string stop1_id = "stop1";
        std::string stop2_id = "stop2";
        std::string stop3_id = "stop3";
        std::string stop4_id = "stop4";

        geo::Coordinates coord1(0.0000, 0.0000);
        geo::Coordinates coord2(10.0000, 0.0000);
        geo::Coordinates coord3(20.0000, 0.0000);
        geo::Coordinates coord4(0.0000, 0.0000);  // = coord1

        catalogue.AddStop(stop1_id, coord1);
        catalogue.AddStop(stop2_id, coord2);
        catalogue.AddStop(stop3_id, coord3);
        catalogue.AddStop(stop4_id, coord4);

        // Проверяем, что остановки созданы
        assert(catalogue.GetStop(stop1_id) != nullptr);
        assert(catalogue.GetStop(stop2_id) != nullptr);
        assert(catalogue.GetStop(stop3_id) != nullptr);
        assert(catalogue.GetStop(stop4_id) != nullptr);

        // GetDistance между остановками
        {
            const Stop* stop1 = catalogue.GetStop(stop1_id);
            const Stop* stop2 = catalogue.GetStop(stop2_id);
            const Stop* stop3 = catalogue.GetStop(stop3_id);

            // Сначала проверяем геодезическое расстояние
            double geo_dist_12 = geo::ComputeDistance(stop1->location, stop2->location);
            double calc_dist_12 = catalogue.GetDistance(stop1, stop2);
            
            assert(std::abs(geo_dist_12 - calc_dist_12) < 0.9);
            
            double geo_dist_23 = geo::ComputeDistance(stop2->location, stop3->location);
            double calc_dist_23 = catalogue.GetDistance(stop2, stop3);
            assert(std::abs(geo_dist_23 - calc_dist_23) < 0.9);
            
            // Расстояние между одинаковыми остановками должно быть 0
            double dist_24 = catalogue.GetDistance(stop1, catalogue.GetStop(stop4_id));
            assert(dist_24 == 0.0);
        }

        // Устанавливаем явные расстояния между остановками 
        catalogue.AddDistance(stop1_id, stop2_id, 15);  // 15 м между stop1 и stop2
        catalogue.AddDistance(stop2_id, stop3_id, 20);  // 20 м между stop2 и stop3
        catalogue.AddDistance(stop3_id, stop2_id, 20);  // 20 м между stop3 и stop2
        catalogue.AddDistance(stop3_id, stop1_id, 20);  // 20 м между stop3 и stop1

        // Проверяем, что явные расстояния возвращаются корректно
        {
            const Stop* stop1 = catalogue.GetStop(stop1_id);
            const Stop* stop2 = catalogue.GetStop(stop2_id);
            const Stop* stop3 = catalogue.GetStop(stop3_id);
                assert(catalogue.GetDistance(stop1, stop2) == 15);
                assert(catalogue.GetDistance(stop2, stop3) == 20);
                assert(catalogue.GetDistance(stop3, stop2) == 20);
                assert(catalogue.GetDistance(stop3, stop1) == 20);
                assert(catalogue.GetDistance(stop2, stop1) == 15);
                assert(catalogue.GetDistance(stop3, stop2) == 20);
                assert(catalogue.GetDistance(stop3, stop1) == 20);
        }

        // Создаём кольцевой маршрут: stop1 → stop2 → stop3 → stop1
        std::vector<std::string_view> route_circular = {
            stop1_id, // 15 
            stop2_id, // 20
            stop3_id, // 20
            stop1_id  
        };            // len = 55
        catalogue.AddBus("bus1", route_circular);

        // Создаём челночный маршрут: stop2 → stop3
        std::vector<std::string_view> route_linear = {
            stop2_id, // 20
            stop3_id, // 20
            stop2_id   
        };            // len = 40
        catalogue.AddBus("bus2", route_linear);

        const Bus* bus1 = catalogue.GetBus("bus1");
        const Bus* bus2 = catalogue.GetBus("bus2");
            assert(bus1 != nullptr);
            assert(bus2 != nullptr);

        // GetRouteLength
        {
            // Для кольцевого маршрута: 
            double length_bus1_fact = catalogue.GetRouteLength(bus1, false);
                //std::cout << "DBG1 fact: " << std::fixed << std::setprecision(10) << length_bus1_fact << "\n";
                assert(std::abs(length_bus1_fact - 55.00) < 0.9);

            // Геодезическая длина кольцевого маршрута — сумма прямых расстояний между соседними остановками
            double length_bus1_geo = catalogue.GetRouteLength(bus1, true);
            double expected_geo_bus1 =
                geo::ComputeDistance(catalogue.GetStop(stop1_id)->location, catalogue.GetStop(stop2_id)->location) +
                geo::ComputeDistance(catalogue.GetStop(stop2_id)->location, catalogue.GetStop(stop3_id)->location) +
                geo::ComputeDistance(catalogue.GetStop(stop3_id)->location, catalogue.GetStop(stop1_id)->location); 
                assert(std::abs(length_bus1_geo - expected_geo_bus1) < 0.9);

            // Для линейного маршрута: только 300 м
            double length_bus2_fact = catalogue.GetRouteLength(bus2, false);
                assert(std::abs(length_bus2_fact - 40.00) < 0.9);
        }

        // GetRouteUniqueStopsCount
        {
            size_t unique_bus1 = catalogue.GetRouteUniqueStopsCount(bus1);
                assert(unique_bus1 == 3);  // stop1, stop2, stop3

            size_t unique_bus2 = catalogue.GetRouteUniqueStopsCount(bus2);
                assert(unique_bus2 == 2);  // stop2, stop3
        }

        // GetBusInfo
        {
            BusInfo info_bus1 = catalogue.GetBusInfo(bus1);
                assert(info_bus1.total_stops == 4);  // stop1→stop2→stop3→stop1
                assert(info_bus1.unique_stops == 3);
                assert(std::abs(info_bus1.length_km - 55.00) < 0.9);
                assert(std::abs(info_bus1.curvature - 0.0000123657) < 0.9);
                
            BusInfo info_bus2 = catalogue.GetBusInfo(bus2);
                assert(info_bus2.total_stops == 3);
                assert(info_bus2.unique_stops == 2);
                assert(std::abs(info_bus2.length_km - 40.00) < 0.9);
                assert(std::abs(info_bus2.curvature - 0.0000179864) < 0.9);
        }

        // Проверка привязки остановок к автобусам
        {
            const Stop* stop2_ptr = catalogue.GetStop(stop2_id);
            const StopBuses* buses_at_stop2 = catalogue.GetStopBuses(stop2_ptr);
                assert(buses_at_stop2 != nullptr);
                assert(buses_at_stop2->size() == 2);
                assert(buses_at_stop2->find("bus1") != buses_at_stop2->end());
                assert(buses_at_stop2->find("bus2") != buses_at_stop2->end());
        }
    }

    std::cout << GREEN << "все тесты функционала пройдены" << RESET << "\n";
}

bool TestFile(const std::string& in_file_name, const std::string& out_file_name) {
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

void TestFiles() {
    bool a = TestFile(".tests/tsC_case0_input.txt", ".tests/tsC_case0_output.txt");
    bool b = TestFile(".tests/tsC_case1_input.txt", ".tests/tsC_case1_output1.txt");

    if (a && b) 
        std::cout << GREEN << "тесты данных пройдены" << RESET << "\n";
    else
        std::cout << RED << "тесты данных не пройдены" << RESET << "\n";
}