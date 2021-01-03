#include "viewer.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/util/dialogs.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/timer.h>


// To have the same shortcut behavior on macOS and other platforms (i.e., Windows, Linux)
#ifdef __APPLE__
#define EASY3D_MOD_CONTROL GLFW_MOD_SUPER
#else
#define EASY3D_MOD_CONTROL GLFW_MOD_CONTROL
#endif


using namespace easy3d;


namespace easy3d {


    ViewerVK::ViewerVK(const std::string &title)
            : title_(title), camera_(nullptr), process_events_(true),
              pressed_button_(-1), modifiers_(-1), drag_active_(false),
              mouse_current_x_(0), mouse_current_y_(0), mouse_pressed_x_(0), mouse_pressed_y_(0), pressed_key_(-1),
              show_pivot_point_(false), model_idx_(-1)
    {
    }


    void ViewerVK::initialize() {
        setup_callbacks(window);

        // create and setup the camera
        camera_ = new Camera;
        camera_->setType(Camera::PERSPECTIVE);
        camera_->setUpVector(vec3(0, 0, 1)); // Z pointing up
        camera_->setViewDirection(vec3(-1, 0, 0)); // X pointing out
        camera_->showEntireScene();
        camera_->connect(this, &ViewerVK::update);

        int fw, fh;
        glfwGetFramebufferSize(window, &fw, &fh);
        // needs to be executed once to ensure the viewer is initialized with correct viewer size
        callback_event_resize(fw, fh);

        /* Poll for events once before starting a potentially lengthy loading process.*/
        glfwPollEvents();

        float xscale, yscale;
        glfwGetWindowContentScale(window, &xscale, &yscale);
        double dpi_scaling = static_cast<double>(xscale + yscale) * 0.5;
        LOG(INFO) << "DPI scaling: " << dpi_scaling;
    }


    void ViewerVK::cleanup() {
        delete camera_;

        for (auto m : models_)
            delete m;
        models_.clear();

        ApplicationVulkan::cleanup();
    }


    void ViewerVK::run() {
        initWindow();
        initVulkan();
        initialize();
        mainLoop();
        cleanup();
    }


    void ViewerVK::mainLoop() {
        static int frame_counter = 0;
        double last_time = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            if (!glfwGetWindowAttrib(window, GLFW_VISIBLE)) // not visible
                continue;

            glfwPollEvents();

            // Calculate ms/frame
            double current_time = glfwGetTime();
            ++frame_counter;
            if(current_time - last_time >= 2.0f) {
                printf("%4.1f ms/frame (fps: %2.0f)\n", 1000.0 / double(frame_counter), double(frame_counter) / (current_time - last_time));
                frame_counter = 0;
                last_time += 2.0f;
            }

            pre_draw();

            drawFrame();

            post_draw();

            /* Wait for mouse/keyboard or empty refresh events */
            glfwWaitEvents();
        }

        /* Process events once more */
        glfwPollEvents();

        vkDeviceWaitIdle(device);
    }


    void ViewerVK::pre_draw() {
     }


    void ViewerVK::post_draw() {

    }


    void ViewerVK::setup_callbacks(GLFWwindow *window) {
        glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;

            int w, h;
            glfwGetWindowSize(win, &w, &h);
            if (x >= 0 && x <= w && y >= 0 && y <= h)
                viewer->callback_event_cursor_pos(x, y);
            else if (viewer->drag_active_) {
                // Restrict the cursor to be within the client area during dragging
                if (x < 0) x = 0;
                if (x > w) x = w;
                if (y < 0) y = 0;
                if (y > h) y = h;
                glfwSetCursorPos(win, x, y);
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int modifiers) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_mouse_button(button, action, modifiers);
        });

        glfwSetKeyCallback(window, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            (void) scancode;
            viewer->callback_event_keyboard(key, action, mods);
        });

        glfwSetCharCallback(window, [](GLFWwindow *win, unsigned int codepoint) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_character(codepoint);
        });

        glfwSetDropCallback(window, [](GLFWwindow *win, int count, const char **filenames) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_drop(count, filenames);
        });

        glfwSetScrollCallback(window, [](GLFWwindow *win, double dx, double dy) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_scroll(dx, dy);
        });

        /* React to framebuffer size events -- includes window size events and also
         * catches things like dragging a window from a Retina-capable screen to a
         * normal screen on Mac OS X */
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int width, int height) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_resize(width, height);
        });

        // notify when the screen has lost focus (e.g. application switch)
        glfwSetWindowFocusCallback(window, [](GLFWwindow *win, int focused) {
            auto viewer = reinterpret_cast<ViewerVK *>(glfwGetWindowUserPointer(win));
            viewer->focus_event(focused != 0);// true for focused
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow *win) {
            glfwSetWindowShouldClose(win, true);
        });
    }


    bool ViewerVK::callback_event_cursor_pos(double x, double y) {
        int px = static_cast<int>(x);
        int py = static_cast<int>(y);
        try {
            int dx = px - mouse_current_x_;
            int dy = py - mouse_current_y_;
            mouse_current_x_ = px;
            mouse_current_y_ = py;
            if (drag_active_)
                return mouse_drag_event(px, py, dx, dy, pressed_button_, modifiers_);
            else
                return mouse_free_move_event(px, py, dx, dy, modifiers_);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool ViewerVK::callback_event_mouse_button(int button, int action, int modifiers) {
        try {
            if (action == GLFW_PRESS) {
                drag_active_ = true;
                pressed_button_ = button;
                modifiers_ = modifiers;
                mouse_pressed_x_ = mouse_current_x_;
                mouse_pressed_y_ = mouse_current_y_;
                return mouse_press_event(mouse_current_x_, mouse_current_y_, button, modifiers);
            } else if (action == GLFW_RELEASE) {
                drag_active_ = false;
                return mouse_release_event(mouse_current_x_, mouse_current_y_, button, modifiers);
            } else {
                drag_active_ = false;
                LOG(INFO) << "GLFW_REPEAT? Seems never happen";
                return false;
            }
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool ViewerVK::callback_event_keyboard(int key, int action, int modifiers) {
        try {
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                return key_press_event(key, modifiers);
            } else {
                return key_release_event(key, modifiers);
            }
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool ViewerVK::callback_event_character(unsigned int codepoint) {
        try {
            return char_input_event(codepoint);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool ViewerVK::callback_event_drop(int count, const char **filenames) {
        try {
            std::vector<std::string> arg(count);
            for (int i = 0; i < count; ++i)
                arg[i] = filenames[i];
            return drop_event(arg);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool ViewerVK::callback_event_scroll(double dx, double dy) {
        try {
            return mouse_scroll_event(mouse_current_x_, mouse_current_y_, static_cast<int>(dx), static_cast<int>(dy));
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    void ViewerVK::callback_event_resize(int w, int h) {
        if (w == 0 && h == 0)
            return;

        try {
            // camera is manipulated by the mouse, working in the screen coordinate system
            int win_w, win_h;
            glfwGetWindowSize(window, &win_w, &win_h);
            camera_->setScreenWidthAndHeight(win_w, win_h);
            post_resize(w, h);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
        }
    }


    void ViewerVK::update() const {
        glfwPostEmptyEvent();
    }


    bool ViewerVK::mouse_press_event(int x, int y, int button, int modifiers) {
        camera_->frame()->action_start();

        if (pressed_key_ == GLFW_KEY_Z || modifiers == GLFW_MOD_SHIFT) {
//            if (button == GLFW_MOUSE_BUTTON_LEFT) {
//                bool found = false;
//                const vec3 &p = point_under_pixel(x, y, found);
//                if (found) {
//                    camera_->setPivotPoint(p);
//                    // show, but hide the visual hint of pivot point after \p delay milliseconds.
//                    show_pivot_point_ = true;
//                    const int delay = 10000;
//                    Timer::single_shot(delay, [&]() {
//                        show_pivot_point_ = false;
//                        update();
//                    });
//
//                    if (pressed_key_ == GLFW_KEY_Z &&  modifiers == 0) {
//#if 1   // with animation
//                        camera()->interpolateToLookAt(p);
//#else   // without animation
//                        const float coef = 0.1f;
//                        const vec3& pos = coef * camera()->frame()->position() + (1.0f - coef)*p;
//                        const quat& ori = camera()->frame()->orientation();
//                        camera_->frame()->setPositionAndOrientation(pos, ori);
//                        camera_->lookAt(p);
//#endif
//                    }
//                } else {
//                    camera_->setPivotPoint(camera_->sceneCenter());
//                    show_pivot_point_ = false;
//                }
//            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
//                camera_->setPivotPoint(camera_->sceneCenter());
//                show_pivot_point_ = false;
//                if (pressed_key_ == GLFW_KEY_Z &&  modifiers == 0) {
//#if 1   // with animation
//                    camera()->interpolateToFitScene();
//#else   // without animation
//                    camera()->showEntireScene();
//#endif
//                }
//            }
        }

        return false;
    }


    bool ViewerVK::mouse_release_event(int x, int y, int button, int modifiers) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && modifiers == EASY3D_MOD_CONTROL) { // ZOOM_ON_REGION
            int xmin = std::min(mouse_pressed_x_, x);
            int xmax = std::max(mouse_pressed_x_, x);
            int ymin = std::min(mouse_pressed_y_, y);
            int ymax = std::max(mouse_pressed_y_, y);
            camera_->fitScreenRegion(xmin, ymin, xmax, ymax);
        } else
            camera_->frame()->action_end();

        pressed_button_ = -1;
        return false;
    }


    bool ViewerVK::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        // control modifier is reserved for zooming on region
        if (modifiers != EASY3D_MOD_CONTROL) {
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    camera_->frame()->action_rotate(x, y, dx, dy, camera_, pressed_key_ == GLFW_KEY_X);
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    camera_->frame()->action_translate(x, y, dx, dy, camera_, pressed_key_ == GLFW_KEY_X);
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    if (std::abs(dy) >= 1)
                        camera_->frame()->action_zoom(dy > 0 ? 1 : -1, camera_);
                    break;
            }
        }

        return false;
    }


    bool ViewerVK::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
        (void) x;
        (void) y;
        (void) dx;
        (void) dy;
        (void) modifiers;
        // highlight geometry primitives here
        return false;
    }


    bool ViewerVK::mouse_scroll_event(int x, int y, int dx, int dy) {
        (void) x;
        (void) y;
        (void) dx;
        camera_->frame()->action_zoom(dy, camera_);
        return false;
    }



    bool ViewerVK::focus_event(bool focused) {
        if (focused) {
            // ...
        }
        return false;
    }

    bool ViewerVK::open() {
        const std::string title("Please choose a file");
        const std::string &default_path = resource::directory() + "/data/";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl)", "*.obj *.ply *.off *.stl",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
                "Polytope Mesh (*.plm)", "*.plm",
                "All Files (*.*)", "*"
        };
        const std::vector<std::string> &file_names = dialog::open(title, default_path, filters, true);

        int count = 0;
        for (const auto &file_name : file_names) {
            if (add_model(file_name))
                ++count;
        }

        if (count > 0) {
            fit_screen();
            return true;
        }
        return false;
    }


    bool ViewerVK::save() const {
        const Model *m = current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return false;
        }

        const std::string &title = "Please choose a file name";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl)", "*.obj *.ply *.off *.stl",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                "Polytope Mesh (*.plm)", "*.plm",
                "All Files (*.*)", "*"
        };

        std::string default_file_name = m->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply"; // default to ply

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        bool saved = false;
        if (dynamic_cast<const PointCloud *>(m))
            saved = PointCloudIO::save(file_name, dynamic_cast<const PointCloud *>(m));
        else if (dynamic_cast<const SurfaceMesh *>(m))
            saved = SurfaceMeshIO::save(file_name, dynamic_cast<const SurfaceMesh *>(m));
        else if (dynamic_cast<const Graph *>(m))
            saved = GraphIO::save(file_name, dynamic_cast<const Graph *>(m));

        if (saved)
            LOG(INFO) << "file successfully saved";

        return saved;
    }


    Model *ViewerVK::add_model(const std::string &file_name, bool create_default_drawables) {
        for (auto m : models_) {
            if (m->name() == file_name) {
                LOG(WARNING) << "model has already been added to the viewer: " << file_name;
                return m;
            }
        }

        const std::string &ext = file_system::extension(file_name, true);
        bool is_ply_mesh = false;
        if (ext == "ply")
            is_ply_mesh = (io::PlyReader::num_instances(file_name, "face") > 0);

        Model *model = nullptr;
        if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "plg") { // mesh
            model = SurfaceMeshIO::load(file_name);
        } else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0) {
            model = GraphIO::load(file_name);
        } else { // point cloud
            if (ext == "ptx") {
                io::PointCloudIO_ptx serializer(file_name);
                PointCloud *cloud = nullptr;
                while ((cloud = serializer.load_next())) {
                    model = add_model(cloud, create_default_drawables);
                    update();
                }
                return model;   // returns the last cloud in the file.
            } else
                model = PointCloudIO::load(file_name);
        }

        if (model) {
            model->set_name(file_name);
            add_model(model, create_default_drawables);
        }
        return model;
    }



    Model *ViewerVK::add_model(Model *model, bool create_default_drawables) {
        if (!model) {
            LOG(WARNING) << "model is NULL.";
            return nullptr;
        }
        for (auto m : models_) {
            if (model == m) {
                LOG(WARNING) << "model has already been added to the viewer: " << m->name();
                return nullptr;
            }
        }

        if (create_default_drawables) {
//            SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(model);
//
//            vertices.reserve(mesh->n_vertices());
//            indices.reserve(mesh->n_vertices());
//
//            auto points = mesh->get_vertex_property<vec3>("v:point");
//            auto texcoords = mesh->get_halfedge_property<vec2>("h:texcoord");
//            auto facecolors = mesh->get_face_property<vec3>("f:color");
//            auto vertexcolors = mesh->get_vertex_property<vec3>("v:color");
//
//            int index = 0;
//            for (auto f : mesh->faces()) {
//                for (auto h : mesh->halfedges(f)) {
//                    auto v = mesh->target(h);
//                    Vertex vertex{};
//                    vertex.pos = points[v];
//
//                    vertex.color = vec3(1.0f, 1.0f, 1.0f);
//                    if (facecolors)
//                        vertex.color = facecolors[f];
//                    else if (vertexcolors)
//                        vertex.color = vertexcolors[v];
//
//                    vertex.texCoord = vec2(0.5f);
//                    if (texcoords)
//                        vertex.texCoord = texcoords[h];
//
//                    vertices.push_back(vertex);
//                    indices.push_back(index++);
//                }
//            }
//
//            vkDestroyBuffer(device, indexBuffer, nullptr);
//            vkFreeMemory(device, indexBufferMemory, nullptr);
//
//            vkDestroyBuffer(device, vertexBuffer, nullptr);
//            vkFreeMemory(device, vertexBufferMemory, nullptr);
//
//            createVertexBuffer();
//            createIndexBuffer();
        }

        int pre_idx = model_idx_;
        models_.push_back(model);
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

        if (model_idx_ != pre_idx) {
            if (model_idx_ >= 0)
                LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
        }
        return model;
    }


    bool ViewerVK::delete_model(Model *model) {
        if (!model) {
            LOG(WARNING) << "model is NULL.";
            return false;
        }

        auto pos = std::find(models_.begin(), models_.end(), model);
        if (pos != models_.end()) {
            int pre_idx = model_idx_;
            const std::string name = model->name();
            models_.erase(pos);
            delete model;
            model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
            LOG(INFO) << "model deleted: " << name;

            if (model_idx_ != pre_idx) {
                if (model_idx_ >= 0)
                    LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
            }
            return true;
        } else {
            LOG(WARNING) << "no such model: " << model->name();
            return false;
        }
    }

    
    Model *ViewerVK::current_model() const {
        if (models_.empty())
            return nullptr;
        if (model_idx_ < models_.size())
            return models_[model_idx_];
        return nullptr;
    }

    bool ViewerVK::key_press_event(int key, int modifiers) {
        if (key == GLFW_KEY_LEFT && modifiers == 0) {
            float angle = static_cast<float>(1 * M_PI / 180.0); // turn left, 1 degrees each step
            camera_->frame()->action_turn(angle, camera_);
        } else if (key == GLFW_KEY_RIGHT && modifiers == 0) {
            float angle = static_cast<float>(1 * M_PI / 180.0); // turn right, 1 degrees each step
            camera_->frame()->action_turn(-angle, camera_);
        } else if (key == GLFW_KEY_UP && modifiers == 0) {    // move camera forward
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, -step)));
        } else if (key == GLFW_KEY_DOWN && modifiers == 0) {// move camera backward
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, step)));
        } else if (key == GLFW_KEY_LEFT && modifiers == EASY3D_MOD_CONTROL) {    // move camera left
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(-step, 0.0, 0.0)));
        } else if (key == GLFW_KEY_RIGHT && modifiers == EASY3D_MOD_CONTROL) {    // move camera right
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(step, 0.0, 0.0)));
        } else if (key == GLFW_KEY_UP && modifiers == EASY3D_MOD_CONTROL) {    // move camera up
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, step, 0.0)));
        } else if (key == GLFW_KEY_DOWN && modifiers == EASY3D_MOD_CONTROL) {    // move camera down
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, -step, 0.0)));
        } else if (key == GLFW_KEY_A && modifiers == 0) {
//            if (drawable_axes_)
//                drawable_axes_->set_visible(!drawable_axes_->is_visible());
        } else if (key == GLFW_KEY_C && modifiers == 0) {
            if (current_model())
                fit_screen(current_model());
        } else if (key == GLFW_KEY_F && modifiers == 0) {
            fit_screen();
        } else if (key == GLFW_KEY_P && modifiers == 0) {
            if (camera_->type() == Camera::PERSPECTIVE)
                camera_->setType(Camera::ORTHOGRAPHIC);
            else
                camera_->setType(Camera::PERSPECTIVE);
        } else if (key == GLFW_KEY_SPACE && modifiers == 0) {
            // Aligns camera
            Frame frame;
            frame.setTranslation(camera_->pivotPoint());
            camera_->frame()->alignWithFrame(&frame, true);

            // Aligns frame
            //if (manipulatedFrame())
            //	manipulatedFrame()->alignWithFrame(camera_->frame());
        } else if (key == GLFW_KEY_O && modifiers == EASY3D_MOD_CONTROL)
            open();
        else if (key == GLFW_KEY_S && modifiers == EASY3D_MOD_CONTROL)
            save();

        else if (key == GLFW_KEY_MINUS && modifiers == EASY3D_MOD_CONTROL)
            camera_->frame()->action_zoom(-1, camera_);
        else if (key == GLFW_KEY_EQUAL && modifiers == EASY3D_MOD_CONTROL)
            camera_->frame()->action_zoom(1, camera_);

        if (key == GLFW_KEY_K && modifiers == GLFW_MOD_ALT) { // add key frame
            easy3d::Frame *frame = camera()->frame();
            camera()->keyframe_interpolator()->add_keyframe(*frame);
            // update scene bounding box to make sure the path is within the view frustum
            float old_radius = camera()->sceneRadius();
            float candidate_radius = distance(camera()->sceneCenter(), frame->position());
            camera()->setSceneRadius(std::max(old_radius, candidate_radius));
        } else if (key == GLFW_KEY_D && modifiers == EASY3D_MOD_CONTROL) { // delete path
            camera()->keyframe_interpolator()->delete_path();

            // update scene bounding box
            Box3 box;
            for (auto m : models_)
                box.add_box(m->bounding_box());
//            for (auto d : drawables_)
//                box.add_box(d->bounding_box());
            camera_->setSceneBoundingBox(box.min(), box.max());
        } else if (key == GLFW_KEY_K && modifiers == EASY3D_MOD_CONTROL) { // play the path
            if (camera()->keyframe_interpolator()->is_interpolation_started())
                camera()->keyframe_interpolator()->stop_interpolation();
            else
                camera()->keyframe_interpolator()->start_interpolation();
        } else if (key == GLFW_KEY_F4 && modifiers == GLFW_MOD_ALT) {
            glfwSetWindowShouldClose(window, true);
        }
        else
            pressed_key_ = key;

        return false;
    }


    bool ViewerVK::key_release_event(int key, int modifiers) {
        (void) key;
        (void) modifiers;
        pressed_key_ = -1;
        return false;
    }


    bool ViewerVK::char_input_event(unsigned int codepoint) {
        (void) codepoint;
        //switch (codepoint) {
        //case '-':	break;
        //case 'c':	break;
        //case 'C':	break;
        //case '0':	break;
        //default:
        //	return false;
        //}

        return false;
    }


    bool ViewerVK::drop_event(const std::vector<std::string> &filenames) {
        int count = 0;
        for (auto &name : filenames) {
            if (add_model(name))
                ++count;
        }

        if (count > 0) {
            model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
            fit_screen();
            return true;
        }
        return false;
    }



    void ViewerVK::fit_screen(const easy3d::Model *model) {
        if (!model && models_.empty())
            return;

        Box3 box;
        if (model)
            box = model->bounding_box();
        else {
            for (auto m : models_)
                box.add_box(m->bounding_box());
        }

        if (box.is_valid()) {
            camera_->setSceneBoundingBox(box.min(), box.max());
            camera_->showEntireScene();
            update();
        }
    }

    void ViewerVK::updateUniformBuffer(uint32_t currentImage) {
        UniformBufferObject ubo{};
        ubo.model = mat4::identity();
        ubo.view = camera_->modelViewMatrix();
        ubo.proj = camera_->projectionMatrix();
        ubo.proj(1, 1) *= -1;

        void *data;
        vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
    }
}
