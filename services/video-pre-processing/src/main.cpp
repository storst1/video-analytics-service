#include <userver/engine/run_starter.hpp>
#include <userver/server/handlers/http_handler.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/logging/log.hpp>
#include <userver/formats/json/serialize.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

// Обработчик HTTP запросов для получения видеопотока
class VideoStreamHandler : public userver::server::handlers::HttpHandler {
public:
    VideoStreamHandler()
        : userver::server::handlers::HttpHandler(userver::server::handlers::HttpHandler::Settings{}) {}

    userver::server::http::HttpResponse HandleRequest(
        const userver::server::http::HttpRequest& request) const override {
        try {
            auto videoUrl = request.GetArg("url");
            if (videoUrl.empty()) {
                return userver::server::http::HttpResponse::BadRequest("Missing 'url' parameter");
            }

            // Открываем видеопоток
            cv::VideoCapture capture(videoUrl);
            if (!capture.isOpened()) {
                return userver::server::http::HttpResponse::BadRequest("Failed to open video stream");
            }

            // Пример обработки: читаем первый кадр
            cv::Mat frame;
            if (capture.read(frame)) {
                // Здесь можно добавить дополнительную обработку кадра
                // Просто отправляем обратно размер полученного кадра
                userver::formats::json::Value result(userver::formats::json::Type::kObject);
                result["width"] = frame.cols;
                result["height"] = frame.rows;
                return userver::server::http::HttpResponse::JsonOk(result);
            }

            return userver::server::http::HttpResponse::InternalServerError("Failed to read from stream");
        } catch (const std::exception& ex) {
            return userver::server::http::HttpResponse::InternalServerError(std::string("Exception caught: ") + ex.what());
        }
    }
};

// Точка входа в приложение
int main(int argc, char* argv[]) {
    userver::engine::RunStarter starter;
    starter.SetHandler<VideoStreamHandler>("/process-video");

    // Запускаем HTTP сервер
    return starter.Run(argc, argv);
}
