#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

int main() {

    using namespace transport_catalogue::input;
    using namespace transport_catalogue::output;

    try {
        // Загрузка запросов
        json::Document request_data {json::Load(std::cin)};

        // Создание загрузчика
        input::JsonReader reader;
        reader.ProcessBaseRequests(request_data);
    
        // Создание отрисовщика
        render::MapRenderer renderer;
        renderer.SetRenderSettings(request_data);

        // Создание ручки
        output::RequestHandler db_handler(
            reader.GetBase(),
            renderer
        );
        
        // Обработка запросов к базе
        json::Document response_data { db_handler.ProcessStatRequests(request_data) };
        
        // Вывод результата
        json::Print(response_data, std::cout);
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}
