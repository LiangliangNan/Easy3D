#include <easy3d/core/model.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/shapes.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/setting.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/file_system.h>

#include "viewer.h" // ensure "gl.h" is included after "glew.h"

using namespace easy3d;

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif


namespace easy3d {


    wxBEGIN_EVENT_TABLE(Viewer, wxGLCanvas)
                    EVT_SIZE(Viewer::OnSize)
                    EVT_PAINT(Viewer::OnPaint)
                    EVT_MOUSE_EVENTS(Viewer::OnMouse)
    wxEND_EVENT_TABLE()

    Viewer::Viewer(wxWindow *parent,
                   wxWindowID id,
                   const wxPoint &pos,
                   const wxSize &size,
                   long style,
                   const wxString &name)
            : wxGLCanvas(parent, id, NULL, pos, size, style | wxFULL_REPAINT_ON_RESIZE, name)
            , initialized_(false)
            , background_color_(0.9f, 0.9f, 1.0f, 1.0f)
            , texter_(nullptr)
            , drawable_axes_(nullptr)
            , model_idx_(-1)
    {
        // Initialize logging (if it has not been initialized yet)
        if (!logging::is_initialized())
            // initialize logging at the very beginning to make sure everything will be logged into the log file.
            logging::initialize(true, true, true, "default", 9);

        const int gl_major = 3;
        const int gl_minor = 2;
        // Explicitly create a new rendering context instance for this canvas.
        wxGLContextAttrs ctxAttrs;
#ifndef __WXMAC__
        // An impossible context, just to test IsOk()
    ctxAttrs.PlatformDefaults().OGLVersion(99, 2).EndList();
    m_oglContext = new wxGLContext(this, NULL, &ctxAttrs);

    if ( !m_oglContext->IsOK() )
    {
#if wxUSE_LOGWINDOW
        wxLogMessage("Trying to set OpenGL 99.2 failed, as expected.");
#endif // wxUSE_LOGWINDOW
        delete m_oglContext;
        ctxAttrs.Reset();
#endif //__WXMAC__
        ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(gl_major, gl_minor).EndList();
        m_glRC = new wxGLContext(this, NULL, &ctxAttrs);
#ifndef __WXMAC__
        }
#endif //__WXMAC__

        if (!m_glRC->IsOK()) {
            std::cerr << "OpenGL version error: This sample needs an OpenGL 3.2 capable driver. The app will end now." << std::endl;
            delete m_glRC;
            m_glRC = NULL;
        }
        else {
//            std::cout << "OpenGL Core Profile 3.2 successfully set." << std::endl;
        }

        // create and setup the camera
        camera_ = new Camera;
        camera_->setType(Camera::PERSPECTIVE);
        camera_->setUpVector(vec3(0, 0, 1)); // Z pointing up
        camera_->setViewDirection(vec3(-1, 0, 0)); // X pointing out
        camera_->showEntireScene();

        easy3d::connect(&camera_->frame_modified, this, &Viewer::update);
    }


    Viewer::~Viewer() {
        delete camera_;
        delete drawable_axes_;
        delete texter_;

        clear_scene();

        ShaderManager::terminate();
        TextureManager::terminate();

        delete m_glRC;
    }


    void Viewer::clear_scene() {
        for (auto m : models_) {
            delete m->renderer();
            delete m;
        }
        models_.clear();
    }


    void Viewer::update() const {
        const_cast<Viewer*>(this)->Refresh();
    }


    void Viewer::OnPaint(wxPaintEvent & WXUNUSED(event)) {
        // must always be here
        wxPaintDC dc(this);

        SetCurrent(*m_glRC);

        // Initialize OpenGL
        if (!initialized_) {
            init();
            initialized_ = true;
        }

        pre_draw();
        draw();
        post_draw();

        // Flush
        glFlush();

        // Swap
        SwapBuffers();
    }


    void Viewer::OnSize(wxSizeEvent & event) {
        // Reset the OpenGL view aspect.
        auto size = event.GetSize();
        const int w = size.GetWidth();
        const int h = size.GetHeight();
        camera_->setScreenWidthAndHeight(w, h);
        glViewport(0, 0, w * dpi_scaling(), h * dpi_scaling());
    }


    void Viewer::OnMouse(wxMouseEvent &event) {
        static bool left_down = false, right_down = false;
        static int prev_x = event.GetX(), prev_y = event.GetY();

        if (event.ButtonDown(wxMOUSE_BTN_ANY)) {
            camera_->frame()->action_start();
            if (event.LeftDown())  left_down = true;
            else if (event.RightDown()) right_down = true;
        }
        else  if (event.ButtonUp(wxMOUSE_BTN_ANY)) {
            camera_->frame()->action_end();
            if (event.LeftUp()) left_down = false;
            else if (event.RightUp()) right_down = false;
        }
        else if (event.Dragging()) {
            const int x = event.GetX();
            const int y = event.GetY();
            const int dx = x - prev_x;
            const int dy = y - prev_y;
            if (left_down)
                camera_->frame()->action_rotate(x, y, dx, dy, camera_, ManipulatedFrame::NONE);
            else if (right_down)
                camera_->frame()->action_translate(x, y, dx, dy, camera_, ManipulatedFrame::NONE);
        }
        else {
            const int rot = event.GetWheelRotation();
            if (rot != 0)
                camera_->frame()->action_zoom(rot > 0 ? 1 : -1, camera_);
        }

        prev_x = event.GetX();
        prev_y = event.GetY();
    }


    void Viewer::init() {
        // Load OpenGL and its extensions
        if (!OpenglUtil::init()) {
            glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
            throw std::runtime_error("failed to load OpenGL and its extensions!");
        }

#ifndef NDEBUG
        opengl::setup_gl_debug_callback();
#endif
        VLOG(1) << "OpenGL vendor: " << glGetString(GL_VENDOR);
        VLOG(1) << "OpenGL renderer: " << glGetString(GL_RENDERER);
//        VLOG(1) << "OpenGL version requested: " << gl_major << "." << gl_minor;
        VLOG(1) << "OpenGL version received: " << glGetString(GL_VERSION);
        VLOG(1) << "GLSL version received: " << glGetString(GL_SHADING_LANGUAGE_VERSION);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glClearDepthf(1.0f);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

        // camera is manipulated by the mouse, working in the screen coordinate system
        // (different from the viewport or framebuffer size, which are in pixel coordinates)
        int w, h;
        GetClientSize(&w, &h);
        camera_->setScreenWidthAndHeight(w, h);
        glViewport(0, 0, w * dpi_scaling(), h * dpi_scaling());

        // create TextRenderer renderer and load default fonts
        texter_ = new TextRenderer(dpi_scaling());
        texter_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
        texter_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");

#if 1
        const std::string file_name = resource::directory() + "/data/easy3d.ply";
        auto model = add_model(file_name);
        if (model) {
            // We always want to look at the front of the easy3d logo.
            camera()->setViewDirection(vec3(0, 0, -1));
            camera()->setUpVector(vec3(0, 1, 0));
            fit_screen(model);
        }
#endif
    }

    void Viewer::fit_screen(const easy3d::Model *model) {
        if (!model && models_.empty()) {
            camera_->showEntireScene();
            return;
        }

        auto visual_box = [](const Model *m) -> Box3 {
            Box3 box = m->bounding_box();
            for (auto d : m->renderer()->points_drawables()) box.grow(d->bounding_box());
            for (auto d : m->renderer()->lines_drawables()) box.grow(d->bounding_box());
            for (auto d : m->renderer()->triangles_drawables()) box.grow(d->bounding_box());
            return box;
        };

        Box3 box;
        if (model)
            box = visual_box(model);
        else {
            for (auto m : models_)
                box.grow(visual_box(m));
        }

        if (box.is_valid()) {
            camera_->setSceneBoundingBox(box.min_point(), box.max_point());
            camera_->showEntireScene();
            update();
        }
    }


    double Viewer::dpi_scaling() const {
        return wxWindow::GetContentScaleFactor();
    }


    Model *Viewer::add_model(const std::string &file_path, bool create_default_drawables) {
        const std::string file_name = file_system::convert_to_native_style(file_path);
        for (auto m: models_) {
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
        if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" ||
            ext == "geojson" || ext == "trilist") { // mesh
            model = SurfaceMeshIO::load(file_name);
        } else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0) {
            model = GraphIO::load(file_name);
        } else if (ext == "plm" || ext == "pm" || ext == "mesh") {
            model = PolyMeshIO::load(file_name);
        } else { // point cloud
            if (ext == "ptx") {
                io::PointCloudIO_ptx serializer(file_name);
                PointCloud *cloud = nullptr;
                while ((cloud = serializer.load_next())) {
                    model = add_model(cloud, create_default_drawables);
                    Refresh();
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


    Model *Viewer::add_model(Model *model, bool create) {
        if (!model) {
            LOG(WARNING) << "model is NULL.";
            return nullptr;
        }
        for (auto m: models_) {
            if (model == m) {
                LOG(WARNING) << "model has already been added to the viewer: " << m->name();
                return nullptr;
            }
        }

        auto renderer = new Renderer(model, create);
        model->set_renderer(renderer);
        auto manipulator = new Manipulator(model);
        model->set_manipulator(manipulator);

        int pre_idx = model_idx_;
        models_.push_back(model);
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

        if (model_idx_ != pre_idx) {
            if (model_idx_ >= 0)
                LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
        }
        return model;
    }



    void Viewer::draw_corner_axes() const {
        ShaderProgram *program = ShaderManager::get_program("surface/surface");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                    ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                    ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"),
                    ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
                    ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
            };
            program = ShaderManager::create_program_from_files("surface/surface", attributes);
        }
        if (!program)
            return;

        if (!drawable_axes_) {
            float base = 0.5f;   // the cylinder length, relative to the allowed region
            float head = 0.2f;   // the cone length, relative to the allowed region
            std::vector<vec3> points, normals, colors;
            shapes::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(base, 0, 0), vec3(1, 0, 0), points, normals, colors);
            shapes::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, base, 0), vec3(0, 1, 0), points, normals, colors);
            shapes::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, 0, base), vec3(0, 0, 1), points, normals, colors);
            shapes::create_cone(0.06, 20, vec3(base, 0, 0), vec3(base + head, 0, 0), vec3(1, 0, 0), points, normals,
                                colors);
            shapes::create_cone(0.06, 20, vec3(0, base, 0), vec3(0, base + head, 0), vec3(0, 1, 0), points, normals,
                                colors);
            shapes::create_cone(0.06, 20, vec3(0, 0, base), vec3(0, 0, base + head), vec3(0, 0, 1), points, normals,
                                colors);
            shapes::create_sphere(vec3(0, 0, 0), 0.06, 20, 20, vec3(0, 1, 1), points, normals, colors);
            const_cast<Viewer*>(this)->drawable_axes_ = new TrianglesDrawable("corner_axes");
            drawable_axes_->update_vertex_buffer(points);
            drawable_axes_->update_normal_buffer(normals);
            drawable_axes_->update_color_buffer(colors);
            drawable_axes_->set_property_coloring(State::VERTEX);
        }
        if (!drawable_axes_->is_visible())
            return;

        // The viewport and the scissor are changed to fit the lower left corner.
        int viewport[4], scissor[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);

        static int corner_frame_size = static_cast<int>(100 * dpi_scaling());
        glViewport(0, 0, corner_frame_size, corner_frame_size);
        glScissor(0, 0, corner_frame_size, corner_frame_size);

        // To make the axis appear over other objects: reserve a tiny bit of the
        // front depth range. NOTE: do remember to restore it later.
        glDepthRangef(0, 0.01f);

        const mat4 &proj = transform::ortho(-1, 1, -1, 1, -1, 1);
        const mat4 &view = camera_->orientation().inverse().matrix();
        const mat4 &MVP = proj * view;

        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera_->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4 &MV = camera_->modelViewMatrix();
        const vec4 &wLightPos = inverse(MV) * setting::light_position;

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("MANIP", mat4::identity())
                ->set_uniform("NORMAL", mat3::identity())   // needs be padded when using uniform blocks
                ->set_uniform("lighting", true)
                ->set_uniform("two_sides_lighting", false)
                ->set_uniform("smooth_shading", true)
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("ssaoEnabled", false)
                ->set_uniform("per_vertex_color", true)
                ->set_uniform("distinct_back_color", false)
                ->set_block_uniform("Material", "ambient", setting::material_ambient)
                ->set_block_uniform("Material", "specular", setting::material_specular)
                ->set_block_uniform("Material", "shininess", &setting::material_shininess)
                ->set_uniform("highlight", false)
                ->set_uniform("clippingPlaneEnabled", false)
                ->set_uniform("selected", false)
                ->set_uniform("use_texture", false);
        drawable_axes_->gl_draw();
        program->release();

        // restore
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glDepthRangef(0.0f, 1.0f);
    }


    void Viewer::pre_draw() {
        glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }


    void Viewer::post_draw() {
        // draw the Easy3D logo and GPU time
        if (texter_ && texter_->num_fonts() >=2) {
            const float font_size = 15.0f;
            const float offset = 20.0f * dpi_scaling();
            texter_->draw("Easy3D", offset, offset, font_size, 0);
        }

        // ------- draw the axes indicating the orientation of the model  ----------
        draw_corner_axes();
    }


    void Viewer::draw() const {
        for (const auto m : models_) {
            if (!m->renderer()->is_visible())
                continue;

            // Let's check if edges and surfaces are both shown. If true, we
            // make the depth coordinates of the surface smaller, so that displaying
            // the mesh and the surface together does not cause Z-fighting.
            std::size_t count = 0;
            for (auto d : m->renderer()->lines_drawables()) {
                if (d->is_visible()) {
                    d->draw(camera_); easy3d_debug_log_gl_error;
                    ++count;
                }
            }

            for (auto d : m->renderer()->points_drawables()) {
                if (d->is_visible())
                    d->draw(camera_); easy3d_debug_log_gl_error;
            }

            if (count > 0) {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(0.5f, -0.0001f);
            }
            for (auto d : m->renderer()->triangles_drawables()) {
                if (d->is_visible())
                    d->draw(camera_); easy3d_debug_log_gl_error;
            }
            if (count > 0)
                glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }
}