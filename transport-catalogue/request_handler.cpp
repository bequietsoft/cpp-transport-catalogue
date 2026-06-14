#include "request_handler.h"
#include "json_builder.h"

namespace transport_catalogue::output {
    
    using namespace transport_catalogue;

    // Обработка запросов к базе
    json::Document RequestHandler::ProcessStatRequests(json::Document& request_data) const {
        json::Array response_root; 
        const auto& request_root = request_data.GetRoot().AsDict();

        if (request_root.count("stat_requests")) {
            const auto& stat_requests = request_root.at("stat_requests").AsArray();

            for (const auto& request_node : stat_requests) {
                const json::Dict& request = request_node.AsDict();
                std::string_view request_type = request.at("type").AsString();
                
                json::Document response;

                if (request_type == "Stop") response = ProcessStopRequest(request);
                if (request_type == "Bus") response = ProcessBusRequest(request);
                if (request_type == "Map") response = ProcessMapRequest(request);
                if (request_type == "Route") response = ProcessRouteRequest(request);

                response_root.push_back(response.GetRoot());
            }
        }

        json::Node response_root_node(response_root);
        return json::Document(response_root_node);
    }

    // Возвращает информацию об автобусе
    std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        const BusPtr bus_ptr = db_.GetBus(bus_name);
        if (!bus_ptr) return std::nullopt;
        return std::move(db_.GetBusStat(bus_ptr));
    }

    // Возвращает автобусы, проходящие через остановку
    const StopBuses* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        const StopPtr stop_ptr = db_.GetStop(stop_name);
        if (!stop_ptr) return nullptr;
        return db_.GetBusesByStop(stop_ptr);
    }

    // Возвращает карту остановок
    svg::Document RequestHandler::GetMap() const {
        return map_renderer_.RenderMap(db_);
	}

    

    // Вспомогательные функции обработки статистических запросов.
    // Вызов из ProcessStatRequests:

    // Запрос информации об остановке
    json::Document RequestHandler::ProcessStopRequest(const json::Dict& request) const {
        json::Builder builder;
        builder.StartDict();

        std::string_view stop_name = request.at("name").AsString();
        const StopBuses* stop_buses_ptr = GetBusesByStop(stop_name);
        if (stop_buses_ptr) {
            builder.Key("buses").StartArray();
            for (auto bus_ptr : *stop_buses_ptr) {
                builder.Value(std::string(bus_ptr->id));
            }
            builder.EndArray();
        } else {
            builder.Key("error_message").Value("not found");
        }

        builder
            .Key("request_id").Value(request.at("id").AsInt())
            .EndDict();
        return builder.Build();
    }

    // Запрос информации об автобусе
    json::Document RequestHandler::ProcessBusRequest(const json::Dict& request) const {
        json::Builder builder;
        builder.StartDict();

        std::string_view bus_name = request.at("name").AsString();
        std::optional<BusStat> bus_stat_opt = GetBusStat(bus_name);
        if (bus_stat_opt.has_value()) {
            const BusStat& stat = bus_stat_opt.value();
            builder
                .Key("curvature").Value(static_cast<double>(stat.curvature))
                .Key("route_length").Value(static_cast<double>(stat.route_length))
                .Key("stop_count").Value(static_cast<int>(stat.stop_count))
                .Key("unique_stop_count").Value(static_cast<int>(stat.unique_stop_count));
        } else {
            builder.Key("error_message").Value("not found");
        }

        builder
            .Key("request_id").Value(request.at("id").AsInt())
            .EndDict();
        return builder.Build();
    }

    // Запрос карты маршрута
    json::Document RequestHandler::ProcessMapRequest(const json::Dict& request) const {
        json::Builder builder;

        builder.StartDict()
            .Key("request_id").Value(request.at("id").AsInt());

        svg::Document map = GetMap();
        std::stringstream string_stream;
        map.Render(string_stream);

        builder.Key("map").Value(string_stream.str());
        
        builder.EndDict();
        return builder.Build();
    }

    // Запрос формирования маршрута
    json::Document RequestHandler::ProcessRouteRequest(const json::Dict& request) const {
        json::Builder builder;
        builder.StartDict();

        int request_id = request.at("id").AsInt();

        std::string_view from = request.at("from").AsString();
        std::string_view to = request.at("to").AsString();
        
        auto info_opt = transport_router_.FindOptimalRoute(from, to);

        if (info_opt.has_value()) {
            const routing::OptimalRouteInfo& info = info_opt.value();

            builder.Key("items").StartArray();
            for (const routing::RouteAction& route_action : info.actions) {
                
                if (std::holds_alternative<routing::WaitAction>(route_action)) {
                    const routing::WaitAction& wait_action = std::get<routing::WaitAction>(route_action);
                    builder
                        .StartDict()
                        .Key("stop_name").Value(std::string(wait_action.stop_id))
                        .Key("time").Value(wait_action.time)
                        .Key("type").Value("Wait")
                        .EndDict();
                } else if (std::holds_alternative<routing::BusAction>(route_action)) {
                    const routing::BusAction& bus_action = std::get<routing::BusAction>(route_action);
                    builder
                        .StartDict()
                        .Key("bus").Value(std::string(bus_action.bus_id))
                        .Key("span_count").Value((int)bus_action.span_count)
                        .Key("time").Value(bus_action.time)
                        .Key("type").Value("Bus")
                        .EndDict();
                }
            }
            builder
                .EndArray()
                .Key("request_id").Value(request_id)
                .Key("total_time").Value(info.total_time);    
        } else {
            builder
                .Key("error_message").Value("not found")
                .Key("request_id").Value(request_id);
        }

        builder.EndDict();        
        return builder.Build();
    }

} // end of namespace transport_catalogue::output