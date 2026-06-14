// Ссылка на урок: [Спринт 13/25: 13. (28.05.2026 - 11.06.2026) 🟢 → Тема 4/5: 
// Рефакторинг → Урок 7/8: Транспортный справочник — поддержка маршрутов]
// ( https://practicum.yandex.ru/trainer/cpp-plus-backend/lesson/4d9611a8-4d19-4d12-83aa-9d3d51567bfe/task/24ead52d-6f4e-4b94-9ceb-2e34245c59b3/ )

#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"


int main() {

    using namespace transport_catalogue::input;
    using namespace transport_catalogue::output;

    try {
        // Загрузка запросов
        json::Document request_data {json::Load(std::cin)};

        // Создание загрузчика
        input::JsonReader reader;

        // Загрузка базы данных
        transport_catalogue::TransportCatalogue db = 
            reader.GetDataBase(request_data);

        // Загрузка параметров маршрутизации
        routing::RoutingSettings routing_settings = 
            reader.GetRoutingSettings(request_data);
        routing::TransportRouter transport_router(routing_settings);

        // Загрузка параметров отрисовщика
        render::RenderSettings render_settings = 
            reader.GetRenderSettings(request_data);
        render::MapRenderer map_renderer(render_settings);

        // Создание ручки
        output::RequestHandler db_handler(db, map_renderer, transport_router);
        
        // Обработка запросов к базе
        json::Document response_data = 
            db_handler.ProcessStatRequests(request_data);
        
        // Вывод результата
        json::Print(response_data, std::cout);
    }
    catch (const std::exception& exception) {
        std::cout << "Error: " << exception.what() << "\n";
        return 1;
    }

    return 0;
}
