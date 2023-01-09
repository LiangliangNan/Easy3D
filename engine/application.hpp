#pragma once

// std
#include <string>
#include <memory>

#include <events.hpp>
#include "frame_rate.h"

namespace lve {

    class Application {
    public:
        explicit Application(const std::string &name = "Application");
        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;
        ~Application();

        /**
         * @brief Enters the main loop of the application.
         * @return An exit code representing the outcome of the loop. EXIT_SUCCESS on success or EXIT_FAILURE on failure.
         */
        virtual int run();

        /**
         * @brief Closes and exits the application.
         */
        void close() const;

        /**
         * @brief Attempts to change the size of the application's window
         * @param width The preferred window width
         * @param height The preferred window height
         */
        void resize(uint32_t width, uint32_t height);

        /**
         * @brief Updates the application
         * @note This function is called in the main loop. Client code can also call it to update the window.
         */
        virtual void update() {}

    protected:
        /**
         * @brief Handles resizing of the window
         * @param width New width of the window
         * @param height New height of the window
         */
        virtual void onResize(uint32_t width, uint32_t height) {}

        /**
         * @brief Handles input key event
         * @param event The input event
         */
        virtual void onKeyEvent(const KeyEvent&) {}

        /**
         * @brief Handles input mouse event
         * @param event The input event
         */
        virtual void onMouseEvent(const MouseEvent&) {}

    protected:
        std::unique_ptr<class LveWindow> window_;

        FrameRate frameRate;

        friend class LveWindow;
     };
}  // namespace lve
