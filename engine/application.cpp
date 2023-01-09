#include "application.hpp"

#include <logging.h>
#include <spdlog/async_logger.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "lve_window.hpp"


namespace lve {

    Application::Application(const std::string &name) {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(name + ".log"));
        auto logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());

#ifdef VULKAN_DEBUG
        logger->set_level(spdlog::level::debug);
#else
        logger->set_level(spdlog::level::info);
#endif

        logger->set_pattern(LOGGER_FORMAT);
        spdlog::set_default_logger(logger);

        LOGI("Logger initialized");

        // const std::vector<Plugin *> plugins{};
        LOGW("ToDo: create plugins available to the platform; also parse the commandline and respond");
        // ...

        // Note: the order of creation matters
        window_ = std::make_unique<LveWindow>(this, name, 800, 600);
    }

    Application::~Application() {
        window_.reset();
        spdlog::drop_all();
    }

    int Application::run() {
        while (!window_->shouldClose()) {
            try {
                if (window_->isVisible() /* && window_->isFocused()*/) {
                    frameRate.check();
                    update();
                }

                window_->processEvents();
            }
            catch (std::exception &e) {
                LOGE("Caught an exception: {}", e.what());
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }


    void Application::close() const {
        window_->close();
    }


    void Application::resize(uint32_t width, uint32_t height) {
        window_->resize(width, height);
    }

}  // namespace lve
