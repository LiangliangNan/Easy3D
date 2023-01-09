#include "lve_window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <unordered_map>

#include <logging.h>
#include <application.hpp>


namespace lve {

    namespace {

        Key translateKey(int key) {
            static const std::unordered_map<int, Key> key_lookup =
                    {
                            {GLFW_KEY_SPACE,         Key::Space},
                            {GLFW_KEY_APOSTROPHE,    Key::Apostrophe},
                            {GLFW_KEY_COMMA,         Key::Comma},
                            {GLFW_KEY_MINUS,         Key::Minus},
                            {GLFW_KEY_PERIOD,        Key::Period},
                            {GLFW_KEY_SLASH,         Key::Slash},
                            {GLFW_KEY_0,             Key::_0},
                            {GLFW_KEY_1,             Key::_1},
                            {GLFW_KEY_2,             Key::_2},
                            {GLFW_KEY_3,             Key::_3},
                            {GLFW_KEY_4,             Key::_4},
                            {GLFW_KEY_5,             Key::_5},
                            {GLFW_KEY_6,             Key::_6},
                            {GLFW_KEY_7,             Key::_7},
                            {GLFW_KEY_8,             Key::_8},
                            {GLFW_KEY_9,             Key::_9},
                            {GLFW_KEY_SEMICOLON,     Key::Semicolon},
                            {GLFW_KEY_EQUAL,         Key::Equal},
                            {GLFW_KEY_A,             Key::A},
                            {GLFW_KEY_B,             Key::B},
                            {GLFW_KEY_C,             Key::C},
                            {GLFW_KEY_D,             Key::D},
                            {GLFW_KEY_E,             Key::E},
                            {GLFW_KEY_F,             Key::F},
                            {GLFW_KEY_G,             Key::G},
                            {GLFW_KEY_H,             Key::H},
                            {GLFW_KEY_I,             Key::I},
                            {GLFW_KEY_J,             Key::J},
                            {GLFW_KEY_K,             Key::K},
                            {GLFW_KEY_L,             Key::L},
                            {GLFW_KEY_M,             Key::M},
                            {GLFW_KEY_N,             Key::N},
                            {GLFW_KEY_O,             Key::O},
                            {GLFW_KEY_P,             Key::P},
                            {GLFW_KEY_Q,             Key::Q},
                            {GLFW_KEY_R,             Key::R},
                            {GLFW_KEY_S,             Key::S},
                            {GLFW_KEY_T,             Key::T},
                            {GLFW_KEY_U,             Key::U},
                            {GLFW_KEY_V,             Key::V},
                            {GLFW_KEY_W,             Key::W},
                            {GLFW_KEY_X,             Key::X},
                            {GLFW_KEY_Y,             Key::Y},
                            {GLFW_KEY_Z,             Key::Z},
                            {GLFW_KEY_LEFT_BRACKET,  Key::LeftBracket},
                            {GLFW_KEY_BACKSLASH,     Key::Backslash},
                            {GLFW_KEY_RIGHT_BRACKET, Key::RightBracket},
                            {GLFW_KEY_GRAVE_ACCENT,  Key::GraveAccent},
                            {GLFW_KEY_ESCAPE,        Key::Escape},
                            {GLFW_KEY_ENTER,         Key::Enter},
                            {GLFW_KEY_TAB,           Key::Tab},
                            {GLFW_KEY_BACKSPACE,     Key::Backspace},
                            {GLFW_KEY_INSERT,        Key::Insert},
                            {GLFW_KEY_DELETE,        Key::DelKey},
                            {GLFW_KEY_RIGHT,         Key::Right},
                            {GLFW_KEY_LEFT,          Key::Left},
                            {GLFW_KEY_DOWN,          Key::Down},
                            {GLFW_KEY_UP,            Key::Up},
                            {GLFW_KEY_PAGE_UP,       Key::PageUp},
                            {GLFW_KEY_PAGE_DOWN,     Key::PageDown},
                            {GLFW_KEY_HOME,          Key::Home},
                            {GLFW_KEY_END,           Key::End},
                            {GLFW_KEY_CAPS_LOCK,     Key::CapsLock},
                            {GLFW_KEY_SCROLL_LOCK,   Key::ScrollLock},
                            {GLFW_KEY_NUM_LOCK,      Key::NumLock},
                            {GLFW_KEY_PRINT_SCREEN,  Key::PrintScreen},
                            {GLFW_KEY_PAUSE,         Key::Pause},
                            {GLFW_KEY_F1,            Key::F1},
                            {GLFW_KEY_F2,            Key::F2},
                            {GLFW_KEY_F3,            Key::F3},
                            {GLFW_KEY_F4,            Key::F4},
                            {GLFW_KEY_F5,            Key::F5},
                            {GLFW_KEY_F6,            Key::F6},
                            {GLFW_KEY_F7,            Key::F7},
                            {GLFW_KEY_F8,            Key::F8},
                            {GLFW_KEY_F9,            Key::F9},
                            {GLFW_KEY_F10,           Key::F10},
                            {GLFW_KEY_F11,           Key::F11},
                            {GLFW_KEY_F12,           Key::F12},
                            {GLFW_KEY_KP_0,          Key::KP_0},
                            {GLFW_KEY_KP_1,          Key::KP_1},
                            {GLFW_KEY_KP_2,          Key::KP_2},
                            {GLFW_KEY_KP_3,          Key::KP_3},
                            {GLFW_KEY_KP_4,          Key::KP_4},
                            {GLFW_KEY_KP_5,          Key::KP_5},
                            {GLFW_KEY_KP_6,          Key::KP_6},
                            {GLFW_KEY_KP_7,          Key::KP_7},
                            {GLFW_KEY_KP_8,          Key::KP_8},
                            {GLFW_KEY_KP_9,          Key::KP_9},
                            {GLFW_KEY_KP_DECIMAL,    Key::KP_Decimal},
                            {GLFW_KEY_KP_DIVIDE,     Key::KP_Divide},
                            {GLFW_KEY_KP_MULTIPLY,   Key::KP_Multiply},
                            {GLFW_KEY_KP_SUBTRACT,   Key::KP_Subtract},
                            {GLFW_KEY_KP_ADD,        Key::KP_Add},
                            {GLFW_KEY_KP_ENTER,      Key::KP_Enter},
                            {GLFW_KEY_KP_EQUAL,      Key::KP_Equal},
                            {GLFW_KEY_LEFT_SHIFT,    Key::LeftShift},
                            {GLFW_KEY_LEFT_CONTROL,  Key::LeftControl},
                            {GLFW_KEY_LEFT_ALT,      Key::LeftAlt},
                            {GLFW_KEY_RIGHT_SHIFT,   Key::RightShift},
                            {GLFW_KEY_RIGHT_CONTROL, Key::RightControl},
                            {GLFW_KEY_RIGHT_ALT,     Key::RightAlt},
                    };

            auto key_it = key_lookup.find(key);

            if (key_it == key_lookup.end()) {
                return Key::Unknown;
            }

            return key_it->second;
        }

        KeyAction translateKeyAction(int action) {
            if (action == GLFW_PRESS) {
                return KeyAction::Down;
            } else if (action == GLFW_RELEASE) {
                return KeyAction::Up;
            } else if (action == GLFW_REPEAT) {
                return KeyAction::Repeat;
            }

            return KeyAction::Unknown;
        }

        MouseButton translateMouseButton(int button) {
            if (button < GLFW_MOUSE_BUTTON_6) {
                return static_cast<MouseButton>(button);
            }
            return MouseButton::Unknown;
        }

        inline MouseAction translateMouseAction(int action) {
            if (action == GLFW_PRESS) {
                return MouseAction::Down;
            } else if (action == GLFW_RELEASE) {
                return MouseAction::Up;
            }

            return MouseAction::Unknown;
        }
    }        // namespace


    LveWindow::LveWindow(Application *app, const std::string &title, uint32_t width, uint32_t height, bool resizable)
        : application{app}
        , title{title}
        , width{width}
        , height{height}
    {
        createWindow(title, resizable);
    }

    LveWindow::~LveWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool LveWindow::shouldClose() const {
        return glfwWindowShouldClose(window);
    }

    void LveWindow::close() const {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    bool LveWindow::isVisible() const {
        return (glfwGetWindowAttrib(window, GLFW_VISIBLE) != 0);
    }

    bool LveWindow::isFocused() const {
        return (glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0);
    }

    void LveWindow::resize(uint32_t width, uint32_t height) {
        glfwSetWindowSize(window, width, height);
    }

    /// @brief It calculates the dpi factor using the density from GLFW physical size
/// <a href="https://www.glfw.org/docs/latest/monitor_guide.html#monitor_size">GLFW docs for dpi</a>
    float LveWindow::get_dpi_factor() const {
        auto primary_monitor = glfwGetPrimaryMonitor();
        auto vidmode = glfwGetVideoMode(primary_monitor);

        int width_mm, height_mm;
        glfwGetMonitorPhysicalSize(primary_monitor, &width_mm, &height_mm);

        // As suggested by the GLFW monitor guide
        static const float inch_to_mm = 25.0f;
        static const float win_base_density = 96.0f;

        auto dpi = static_cast<uint32_t>(vidmode->width / (width_mm / inch_to_mm));
        auto dpi_factor = dpi / win_base_density;
        return dpi_factor;
    }

    float LveWindow::get_content_scale_factor() const {
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        int win_width, win_height;
        glfwGetWindowSize(window, &win_width, &win_height);

        // We could return a 2D result here instead of a scalar,
        // but non-uniform scaling is very unlikely, and would
        // require significantly more changes in the IMGUI integration
        return static_cast<float>(fb_width) / win_width;
    }

    void LveWindow::processEvents() {
        static bool want_frame_rate = true;
        if (want_frame_rate)
            glfwPollEvents();
        else
            glfwWaitEvents();
    }

    void LveWindow::createWindow(const std::string& title, bool resizable) {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW couldn't be initialized.");
        }

        glfwSetErrorCallback([](int error, const char *description) {
            LOGE("GLFW Error (code {}): {}", error, description);
        });

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(resizable));

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Couldn't create glfw window.");
        }

        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
            auto win = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
            win->framebufferResized = true;
            win->width = width;
            win->height = height;
            win->application->onResize(width, height);
        });
        glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/) {
            auto win = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
            win->application->onKeyEvent(KeyEvent{lve::translateKey(key), lve::translateKeyAction(action)});
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
            auto win = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
            /// todo: resolve mouse button/pos/dx/dy in the Window callback.
            win->application->onMouseEvent(MouseEvent{
                    MouseButton::Unknown,
                    MouseAction::Move,
                    static_cast<float>(xpos),
                    static_cast<float>(ypos)});
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int /*mods*/) {
            auto win = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            win->application->onMouseEvent(MouseEvent{
                    translateMouseButton(button),
                    translateMouseAction(action),
                    static_cast<float>(xpos),
                    static_cast<float>(ypos)});
        });
    }

    VkSurfaceKHR LveWindow::createSurface(VkInstance instance) const {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
        return surface;
    }

    std::vector<const char *> LveWindow::getInstanceExtensions() const {
        uint32_t count = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
        return std::vector<const char *>(glfwExtensions, glfwExtensions + count);
    }

}  // namespace lve
