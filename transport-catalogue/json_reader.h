#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport_catalogue::input {

    using namespace transport_catalogue;
    using namespace render;

    class JsonReader {
    public:
        // Обрабатывает запросы формирования базы
        TransportCatalogue GetDataBase(const json::Document& request_data) const;
    
        // Читает параметры отрисовки
        RenderSettings GetRenderSettings(const json::Document& request_data) const;
    };

} // end of namespace transport_catalogue::input