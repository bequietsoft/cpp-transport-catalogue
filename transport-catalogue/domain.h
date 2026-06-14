#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>
#include <chrono>

namespace domain {

    struct Stop;
    struct Bus;
    struct BusStat;
    struct StopBusesComparator;

    using StopPtr = Stop*;
    using BusPtr = Bus*;

    using Buses = std::vector<BusPtr>;
    using Stops = std::vector<StopPtr>;
    
    using StopBuses = std::set<BusPtr, StopBusesComparator>;

    struct Stop {
        std::string id;
        transport_catalogue::geo::Coordinates location;

        Stop(const std::string id, transport_catalogue::geo::Coordinates location)
            : id(std::move(id)), location(location) {}
    };

    struct Bus {
        std::string id;
        Stops route;
        bool is_roundtrip;

        Bus(const std::string id, Stops route, bool is_roundtrip)
            : id(std::move(id)), route(std::move(route)), is_roundtrip(is_roundtrip) {}
    };

    struct BusStat {
        size_t stop_count;
        size_t unique_stop_count;
        double route_length;
        double curvature;
    };

    struct StopBusesComparator {
        bool operator()(const BusPtr& a, const BusPtr& b) const { 
            return a->id < b->id; 
        }
    };

} // end of namespace domain