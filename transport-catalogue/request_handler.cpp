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

                response_root.push_back(response.GetRoot());
            }
        }

        json::Node response_root_node(response_root);
        return json::Document(response_root_node);
    }

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        const BusPtr bus_ptr = db_.GetBus(bus_name);
        if (!bus_ptr) return std::nullopt;
        return std::move(db_.GetBusStat(bus_ptr));
    }

    // Возвращает маршруты, проходящие через остановку (запрос Stop)
    const StopBuses* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        const StopPtr stop_ptr = db_.GetStop(stop_name);
        if (!stop_ptr) return nullptr;
        return db_.GetBusesByStop(stop_ptr);
    }

    // Отрисовка карты маршрута в SVG
    svg::Document RequestHandler::GetMap() const {
        return renderer_.RenderMap(db_);
	}

    // Запрос информации об остановке
    json::Document RequestHandler::ProcessStopRequest(const json::Dict& request) const {
        json::Builder builder;
        builder.StartDict()
            .Key("request_id").Value(request.at("id").AsInt());

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

        builder.EndDict();
        return builder.Build();
    }

    // Запрос информации о маршруте
    json::Document RequestHandler::ProcessBusRequest(const json::Dict& request) const {
        json::Builder builder;
        builder.StartDict()
            .Key("request_id").Value(request.at("id").AsInt());

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

        builder.EndDict();
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

} // end of namespace transport_catalogue::output