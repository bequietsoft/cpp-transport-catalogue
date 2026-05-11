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

        // Загрузка базы данных
        transport_catalogue::TransportCatalogue db = 
            reader.GetDataBase(request_data);

        // Загрузка параметров отрисовщика
        render::RenderSettings render_settings = 
            reader.GetRenderSettings(request_data);

        render::MapRenderer renderer(render_settings);

        // Создание ручки
        output::RequestHandler db_handler(db, renderer);
        
        // Обработка запросов к базе
        json::Document response_data = db_handler.ProcessStatRequests(request_data);
        
        // Вывод результата
        json::Print(response_data, std::cout);
    }
    catch (const std::exception& exception) {
        std::cout << "Error: " << exception.what() << "\n";
        return 1;
    }

    return 0;
}
