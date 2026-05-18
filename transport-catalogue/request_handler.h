#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"

#include <memory>
#include <optional>
#include <string>

namespace transport_catalogue::output {
    
    using namespace transport_catalogue;

    class RequestHandler {
    public:
        // Конструктор
        explicit RequestHandler(const TransportCatalogue& db, const render::MapRenderer& renderer) 
            : db_(db), renderer_(renderer) {}

        // Обработка запросов к каталогу
        json::Document ProcessStatRequests(json::Document& request_data) const;

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через остановку (запрос Stop)
        const StopBuses* GetBusesByStop(const std::string_view& stop_name) const;

        // Отрисовка карты маршрута в SVG
        svg::Document GetMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& db_;
        const render::MapRenderer& renderer_;

        // Запрос информации об остановке
        json::Document ProcessStopRequest(const json::Dict& request) const;

        // Запрос информации о маршруте
        json::Document ProcessBusRequest(const json::Dict& request) const;

        // Запрос карты маршрута
        json::Document ProcessMapRequest(const json::Dict& request) const;
    };

} // end of namespace transport_catalogue::output
