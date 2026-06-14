#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "json.h"

#include <memory>
#include <variant>
#include <optional>
#include <string>

namespace transport_catalogue::output {
    
    using namespace transport_catalogue;

    class RequestHandler {
    public:
        // Конструктор
        explicit RequestHandler(
            const TransportCatalogue& db, 
            const render::MapRenderer& map_renderer,
            const routing::TransportRouter& transport_router) 
            : db_(db), map_renderer_(map_renderer), transport_router_(transport_router) {}

        // Обработка запросов к базе
        json::Document ProcessStatRequests(json::Document& request_data) const;

        // Возвращает информацию об автобусе
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает автобусы, проходящие через остановку
        const StopBuses* GetBusesByStop(const std::string_view& stop_name) const;

        // Возвращает карту остановок
        svg::Document GetMap() const;

    private:
        // RequestHandler использует агрегацию объектов:
        // "Транспортный Справочник", "Визуализатор Карты", "Транспортный Маршрутизатор"
        const TransportCatalogue& db_;
        const render::MapRenderer& map_renderer_;
        const routing::TransportRouter& transport_router_;


        // Вспомогательные функции обработки статистических запросов.
        // Вызов из ProcessStatRequests:

        // Запрос информации об остановке
        json::Document ProcessStopRequest(const json::Dict& request) const;

        // Запрос информации об автобусе
        json::Document ProcessBusRequest(const json::Dict& request) const;

        // Запрос карты остановок
        json::Document ProcessMapRequest(const json::Dict& request) const;

        // Запрос формирования маршрута
        json::Document ProcessRouteRequest(const json::Dict& request) const;
    };

} // end of namespace transport_catalogue::output
