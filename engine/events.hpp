#pragma once


namespace lve {

    enum class EventSource {
        Keyboard,
        Mouse,
        Touchscreen
    };

    class Event {
    public:
        Event(EventSource source);
        EventSource get_source() const;

    private:
        EventSource source;
    };

    enum class Key {
        Unknown,
        Space,
        Apostrophe, /* ' */
        Comma,      /* , */
        Minus,      /* - */
        Period,     /* . */
        Slash,      /* / */
        _0,
        _1,
        _2,
        _3,
        _4,
        _5,
        _6,
        _7,
        _8,
        _9,
        Semicolon, /* ; */
        Equal,     /* = */
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LeftBracket,  /* [ */
        Backslash,    /* \ */
        RightBracket, /* ] */
        GraveAccent,  /* ` */
        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        DelKey,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        Back,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        KP_0,
        KP_1,
        KP_2,
        KP_3,
        KP_4,
        KP_5,
        KP_6,
        KP_7,
        KP_8,
        KP_9,
        KP_Decimal,
        KP_Divide,
        KP_Multiply,
        KP_Subtract,
        KP_Add,
        KP_Enter,
        KP_Equal,
        LeftShift,
        LeftControl,
        LeftAlt,
        RightShift,
        RightControl,
        RightAlt
    };

    /// todo: add a "Modifier"

    enum class KeyAction {
        Down,
        Up,
        Repeat,
        Unknown
    };

    class KeyEvent : public Event {
    public:
        KeyEvent(Key code, KeyAction action);
        Key get_code() const;
        KeyAction get_action() const;

    private:
        Key code;
        KeyAction action;
    };

    enum class MouseButton {
        Left,
        Right,
        Middle,
        Back,
        Forward,
        Unknown
    };

    enum class MouseAction {
        Down,
        Up,
        Move,
        Unknown
    };

    class MouseEvent : public Event {
    public:
        MouseEvent(MouseButton button, MouseAction action, float pos_x, float pos_y);
        MouseButton get_button() const;
        MouseAction get_action() const;

        float get_pos_x() const;
        float get_pos_y() const;

    private:
        MouseButton button;
        MouseAction action;
        float pos_x;
        float pos_y;
    };


//    struct KeyMappings;
//    class CameraController {
//    public:
//        struct KeyMappings {
//            Key moveLeft = Key::A;
//            Key moveRight = Key::D;
//            Key moveForward = Key::W;
//            Key moveBackward = Key::S;
//            Key moveUp = Key::E;
//            Key moveDown = Key::Q;
//            Key lookLeft = Key::Left;
//            Key lookRight = Key::Right;
//            Key lookUp = Key::Up;
//            Key lookDown = Key::Down;
//        };
//
//        void moveInPlaneXZ(GLFWwindow *window, float dt, LveGameObject &gameObject);
//
//        KeyMappings* keys{};
//        float moveSpeed{3.f};
//        float lookSpeed{1.5f};
//    };
}  // namespace lve