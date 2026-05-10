#pragma once

#include "transport_catalogue.h"
#include "json.h"

namespace transport_catalogue::input {

    using namespace transport_catalogue;
    
    class JsonReader {
    public:
        // Обрабатывает запросы формирования базы
        void ProcessBaseRequests(json::Document& request_data);
    
        // Доступ к базе для чтения
        const TransportCatalogue& GetBase() const { return db_; }
        
    private:
        TransportCatalogue db_ {};

        // Обрабатывает запрос создания остановки
        void ProcessStopRequest(const json::Dict& request);

        // Обрабатывает запрос создания маршрута
        void ProcessBusRequest(const json::Dict& request);
    };

} // end of namespace transport_catalogue::input