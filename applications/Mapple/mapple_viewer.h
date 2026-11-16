/** \cond LICENSE
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */

#ifndef EASY3D_MAPPLE_VIEWER_H
#define EASY3D_MAPPLE_VIEWER_H

#include <atomic>
#include <chrono>
#include <cstddef>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/state.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/progress.h>
#include <easy3d/viewer/viewer.h>

#include "settings.h"

class ImGuiLayer;
namespace easy3d {
    class AmbientOcclusion;
    class Drawable;
    class EyeDomeLighting;
    class Model;
    class PointCloudPicker;
    class SoftShadow;
    class SurfaceMeshPicker;
    class Texture;
    class Transparency;
    class VideoEncoder;
    namespace graphics { struct DrawCommand; }
}


/**
 * \brief The Mapple shell: a Viewer with a menu bar, toolbar, docked-style panels and a log sink.
 * \details The class owns the ImGui context (through ImGuiLayer), routes window input through
 *      ImGui first, lays out the panels around the central 3D scene, and collects Easy3D log
 *      messages into the Log panel. See docs/mapple.md for the port plan.
 */
class MappleViewer : public easy3d::Viewer, public easy3d::logging::Logger, public easy3d::ProgressClient {
public:
    explicit MappleViewer(
            const std::string &title = "Mapple",
            int width = 1440,
            int height = 900,
            int samples = 4,
            bool full_screen = false,
            bool visible = true,
            bool resizable = true,
#ifdef EASY3D_HAS_VULKAN
            const easy3d::VulkanOptions &options = {}
#else
            const easy3d::OpenGLOptions &options = {}
#endif
    );

    ~MappleViewer() override;

    /// Receives every Easy3D log message; shown in the Log panel.
    void send(el::Level level, const std::string &msg) override;

    /// Receives progress from a background command; called on the worker thread, so
    /// it only stores the percentage for the progress window.
    void notify(std::size_t percent, bool update_viewer) override;

    /// Adds the model and attaches a manipulator, so the Edit commands can move and
    /// rotate it and the drawables apply the resulting transform.
    easy3d::Model *add_model(std::shared_ptr<easy3d::Model> model, bool create_drawables = true,
                             bool fit_scene = true) override;

    // The raw-pointer and file-name forms stay visible; both forward to the override.
    using easy3d::Viewer::add_model;

protected:
    void init() override;

    void pre_draw() override;

    // Wraps the scene render in the enabled effect (ambient occlusion, eye dome
    // lighting, soft shadow, or transparency); snapshots and recordings go through
    // this too, so they capture the effect.
    void draw() const override;

    void post_resize(int w, int h) override;

    // Input hooks: return true while ImGui owns the input, so the gesture does not
    // also navigate the 3D scene.
    bool mouse_press_event(int x, int y, int button, int modifiers) override;

    bool mouse_release_event(int x, int y, int button, int modifiers) override;

    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;

    bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) override;

    bool mouse_scroll_event(int x, int y, int dx, int dy) override;

    bool key_press_event(int key, int modifiers) override;

    bool key_release_event(int key, int modifiers) override;

    bool char_input_event(unsigned int codepoint) override;

    bool focus_event(bool focused) override;

private:
    struct LogLine {
        el::Level level;
        std::string text;
    };

    // How a pointer press selects primitives: click, rubber band, or lasso.
    enum class SelectMode { OFF, CLICK, RECT, LASSO };

    // What a finished selection gesture does with the primitives it touches.
    enum class SelectAction { REPLACE, ADD, REMOVE };

    // Selection commands that touch drawable buffers; queued from the keyboard.
    enum class SelectCommand { NONE, SELECT_ALL, CLEAR_SELECTION, INVERT_SELECTION, DELETE_SELECTION };

    // The processing command whose parameter window is open. Each command edits the
    // active model in place and reports through the Log panel.
    enum class ProcessOp {
        NONE,
        PC_ESTIMATE_NORMALS, PC_REORIENT_NORMALS, PC_NORMALIZE_NORMALS, PC_SIMPLIFY,
        PC_POISSON, PC_RANSAC, PC_DELAUNAY_2D, PC_DELAUNAY_3D,
        MESH_SMOOTH, MESH_SIMPLIFY, MESH_REMESH, MESH_FILL_HOLES, MESH_POLYGONIZE,
        MESH_FAIRING, MESH_SAMPLE,
        MESH_COMPONENTS, MESH_PLANAR_PARTITION, MESH_TRIANGULATE, MESH_DUAL,
        MESH_SUBDIVIDE_CC, MESH_SUBDIVIDE_LOOP, MESH_SUBDIVIDE_SQRT3,
        MESH_PARAMETERIZE, MESH_GEODESIC,
        MESH_REPAIR_SOUP, MESH_ORIENT_STITCH_SOUP, MESH_STITCH_REORIENT, MESH_STITCH,
        MESH_ORIENT_OUTWARD, MESH_REVERSE_ORIENTATION,
        MESH_REMOVE_ISOLATED, MESH_REMOVE_DUPLICATE_FACES,
        MESH_DETECT_SELF_INTERSECTIONS, MESH_REMESH_SELF_INTERSECTIONS,
        MESH_CLIP, MESH_SPLIT, MESH_SLICE,
        MESH_TETRAHEDRALIZE, MESH_CREATE_FROM_TEXT,
        POLY_EXTRACT_BOUNDARY,
        EDIT_GAUSSIAN_NOISE,
        PROP_HEIGHT_FIELD, PROP_CURVATURES, PROP_AMBIENT_OCCLUSION, PROP_TOPOLOGY
    };

    // Parameters of the open processing window. Model-dependent values are refreshed
    // when the window opens.
    struct SnapshotParams {
        float scale = 1.0f;         // image size = framebuffer size * scale
        int samples = 4;
        int background = 0;         // 0: current color, 1: white, 2: transparent
        bool expand = true;         // expand the frustum to the output aspect ratio
    };

    struct ProcessParams {
        int normals_k = 16;
        bool compute_curvature = false;
        int point_target = 0;

        int poisson_depth = 8;
        float poisson_samples = 1.0f;
        bool poisson_trim = false;
        float poisson_trim_value = 6.0f;
        float poisson_island_ratio = 0.001f;

        bool ransac_plane = true;
        bool ransac_sphere = true;
        bool ransac_cylinder = true;
        bool ransac_cone = false;
        bool ransac_torus = false;
        int ransac_min_support = 1000;
        float ransac_distance = 0.005f;
        float ransac_bitmap = 0.02f;
        float ransac_normal = 0.8f;
        float ransac_overlook = 0.001f;
        bool ransac_selected_only = false;

        bool smooth_implicit = false;
        bool smooth_uniform = false;
        int smooth_iterations = 10;
        int simplify_target = 0;
        float remesh_edge_factor = 1.0f;
        int hole_max_size = 500;
        float polygonize_angle = 1.0f;
        int fairing_criterion = 0;      // 0: area, 1: curvature, 2: curvature variation
        int sample_points = 1000000;
        int param_method = 0;           // 0: LSCM, 1: discrete harmonic
        bool param_uniform_weights = false;
        bool param_preview = true;

        int curvature_smoothing = 1;    // post-smoothing iterations of the curvature values
        bool curvature_two_ring = true;

        float noise_sigma = 0.001f;     // standard deviation; the Qt dialog's default

        char text_buffer[128] = "Easy3D"; // TextMesher input
        std::string font_file;          // full path to the .ttf; set when the window opens
        int text_font_size = 45;
        int text_extrusion = 10;
        int text_bezier_steps = 4;
        bool text_collision_free = true;

        float ao_radius_ratio = 0.1f;   // search radius as a fraction of the bounding-sphere radius
        int ao_samples = 128;           // cosine-weighted hemisphere rays per vertex
        float ao_bias_ratio = 0.001f;   // ray origin offset, as a fraction of the search radius
    };

    // The drawable appearance in effect before the selection view replaced it, so
    // clearing the selection restores the user's appearance instead of leaving the
    // model painted with the selection colormap.
    struct SelectionStyle {
        easy3d::State state;                        // full drawable state before selection
        std::unique_ptr<easy3d::Texture> colormap;  // 2x1: unselected color -> highlight
    };

    // How a drawable's scalar field is colored. The coloring itself lives in the
    // drawable's state; this keeps the Rendering panel's controls populated across
    // frames and renderer updates.
    struct ScalarFieldStyle {
        int colormap = 0;           // index into the colormap list in ui/panels.cpp
        bool discrete = false;
        int stripes = 16;
        bool clamp = true;          // ignore the value percentiles below when false
        float clamp_lower = 0.05f;
        float clamp_upper = 0.05f;
    };

    // Framebuffer-to-window ratio; may differ from dpi_scaling().
    float pixel_ratio() const;

    void draw_gui();

    // ---- rendering effects (ui/panels.cpp) ----

    // The effects are full-frame passes that own the scene render, so only one runs
    // at a time: enabling one turns the others off (the Qt app's Global Setting panel
    // enforced the same rules between shadow, transparency, AO and EDL).
    void set_ao_enabled(bool enabled);

    void set_edl_enabled(bool enabled);

    void set_shadow_enabled(bool enabled);

    void set_transparency_enabled(bool enabled);

    bool ao_enabled() const { return ao_ != nullptr; }

    bool edl_enabled() const { return edl_ != nullptr; }

    bool shadow_enabled() const { return shadow_ != nullptr; }

    bool transparency_enabled() const { return transparency_ != nullptr; }

    // Pushes the effect parameters in settings_ into the live effects and redraws.
    void apply_effect_parameters();

    // ui/menu.cpp
    void draw_menu_bar();

    // ui/menu.cpp
    void draw_menu_edit();

    // Subtracts the first model's bounding-box center from every model, then fits the
    // scene. Models are stored in local coordinates, so this recenters the whole scene
    // without touching the camera's existing pose.
    void translational_recenter();

    // Bakes each model's manipulator transform into its points (and normals) and
    // resets the manipulator.
    void apply_manipulated_transformation();

    // Resets the current model's manipulator, discarding the live transform.
    void discard_manipulated_transformation();

    // ui/menu.cpp
    void draw_menu_select();

    // ui/menu.cpp
    void draw_menu_process();

    // ui/menu.cpp
    void draw_menu_property();

    // ui/process.cpp

    void open_process(ProcessOp op);

    // Draws the parameter window of the open processing command; input callbacks keep
    // the scene navigation away from it while it is open.
    void draw_process_dialog();

    void apply_process();

    // Runs a command that needs no parameters directly (no parameter window).
    void run_process(ProcessOp op);

    // Commands heavy enough to run on a worker thread. The algorithm mutates the
    // model off the main thread; everything that touches drawables, renderers or the
    // graphics context happens in finalize_process() after the thread joins.
    struct ProcessJob {
        std::thread thread;
        std::atomic<bool> done{false};
        std::atomic<int> percent{-1};   // -1 until the algorithm reports progress
        bool cancelable = false;        // the algorithm polls ProgressLogger::is_canceled()
        bool cancel_requested = false;
        bool failed = false;            // the algorithm threw; it was logged in the thread
        easy3d::Model *target = nullptr; // the model the command edits
        easy3d::Model *result = nullptr; // new model handed to the viewer on finalize
        std::chrono::steady_clock::time_point start;
    };

    static bool is_long_running(ProcessOp op);

    // The command rebuilds the model, so every property (including the selection)
    // is discarded and the stored selection style no longer applies.
    static bool discards_model_properties(ProcessOp op);

    // The former body of apply_process(): runs the command, returns whether the
    // selection display must be left alone afterwards. May run on a worker thread,
    // so it must not touch GPU resources or add models directly.
    bool run_process_algorithm(easy3d::Model *model);

    void start_process_job();

    // Joins a finished job and adopts its result model on the main thread.
    void poll_process_job();

    void finalize_process(easy3d::Model *model, std::chrono::steady_clock::time_point start,
                          bool skip_selection_display);

    void request_process_cancel();

    // Modal progress window shown while a background command runs.
    void draw_process_progress();

    // ui/properties.cpp

    // Opens the floating Properties window on the active model.
    void open_properties_window();

    // Floating, non-modal property editor: lists the model's vertex, face, edge and
    // halfedge properties; right-clicking one removes, renames, converts or colors
    // by it. The scene stays interactive while it is open.
    void draw_properties_window();

    // Points, lines and triangles drawables (and the appearance saved by the
    // selection display) that colored by old_name now color by new_name; an empty
    // new_name resets them to uniform coloring. Keeps the view valid instead of
    // leaving stale property references.
    void sync_property_references(easy3d::Model *model, easy3d::State::Location location,
                                  const std::string &old_name, const std::string &new_name);

    // Colors the drawable that draws the given location by the color property, so a
    // generated color property is visible immediately. While a selection is shown,
    // the colors become the appearance restored when the selection ends.
    void apply_color_property(easy3d::Model *model, easy3d::State::Location location,
                              const std::string &name);

    // ui/snapshot.cpp
    void open_snapshot_dialog();

    void draw_snapshot_dialog();

    // ui/animation.cpp

    // Floating, non-modal window: keyframe count and navigation, playback, and the
    // keyframe interpolation settings. The scene stays interactive so the user can
    // move the camera and add keyframes while it is open.
    void open_animation_window();

    void draw_animation_window();

    void add_camera_keyframe();

    void remove_last_camera_keyframe();

    void clear_camera_path();

    void toggle_camera_path_playback();

    // Stops playback and moves the camera to a keyframe.
    void goto_camera_keyframe(int index);

    // Shows/hides the camera path and keyframe cameras, widening the scene radius
    // while visible so the whole path stays in the depth range.
    void set_camera_path_visible(bool visible);

    void import_camera_path();

    void export_camera_path();

    // Video recording needs FFmpeg; without it recording_video() is always false.
#ifdef HAS_FFMPEG
    // One camera-path video. The frames are rendered through Viewer::capture() one
    // per drawn frame, so the progress window reacts and can cancel; the encoder
    // finalizes the stream when it is destroyed.
    struct VideoRecording {
        std::unique_ptr<easy3d::VideoEncoder> encoder;
        std::vector<easy3d::Frame> frames;   // interpolated path, rendered in order
        std::size_t next_frame = 0;          // index of the frame rendered next
        int width = 0, height = 0;           // encoded size (multiples of 8)
        int capture_width = 0, capture_height = 0; // view size when recording started
        unsigned samples = 1;
        std::vector<unsigned char> pixels;   // cropped frame handed to the encoder
        std::string file_name;
        std::chrono::steady_clock::time_point start;
        easy3d::Frame camera;                // camera state restored when it ends
        bool snapshot_hud = true;
        bool cancel_requested = false;
        // Out of line: VideoEncoder is only forward declared in this header.
        ~VideoRecording();
    };
#endif

    // True while a camera-path video is being recorded.
    bool recording_video() const;

    // Opens the save dialog and starts recording the camera path (no-op without
    // FFmpeg and with fewer than two keyframes).
    void record_camera_path();

    // Renders and encodes one video frame per drawn frame; called from pre_draw()
    // so the scene is rendered outside draw_gui() and the UI stays responsive.
    void advance_video_recording();

    // Modal progress window of the running recording.
    void draw_record_progress();

    // Asks the running recording to stop; the frames written so far are kept.
    void cancel_video_recording();

    // ui/menu.cpp
    // App-owned file dialogs: Viewer's start in resources/data and know nothing
    // about recent files. These start in the last-used directory and remember
    // every successfully opened or saved file.
    bool open() override;

    bool save() const override;

    // Adds a file chosen from File > Open Recent and fits the camera to it.
    void open_recent_file(const std::string &file);

    // ui/panels.cpp
    void draw_toolbar(float x, float y, float w, float h);

    void draw_model_info(easy3d::Model *model);

    void draw_panels(float x, float y, float w, float h);

    void draw_rendering_panel(float x, float y, float w, float h);

    // Color scheme combo of one drawable (uniform color, a color property, texture
    // coordinates, or a scalar field) and, for scalar fields, the colormap controls.
    void draw_color_scheme(easy3d::Model *model, easy3d::Drawable *drawable);

    // The texture of a scalar field style, from the colormap list in ui/panels.cpp.
    easy3d::Texture *scalar_texture(const ScalarFieldStyle &style) const;

    // Re-applies a scalar field style to the named property. The property must be
    // passed explicitly: the drawable's own name is reset when it switches to
    // uniform coloring, so it cannot be used to return to the previous scalar field.
    void apply_scalar_style(easy3d::Drawable *drawable, easy3d::State::Location location,
                            const std::string &property, const ScalarFieldStyle &style);

    void draw_models_panel(float x, float y, float w, float h);

    void draw_log_panel(float x, float y, float w, float h);

    void draw_status_bar(float x, float y, float w, float h);

    void draw_about_window();

    // A 4 px drag handle between the scene and a panel. dragging along the axis
    // adjusts *value by sign * mouse_delta.
    void draw_splitter(const char *id, float x, float y, float w, float h,
                       bool resize_width, float *value, float sign,
                       float min_value, float max_value);

    void delete_model_confirm(easy3d::Model *model);

    // ---- selection (mapple_viewer.cpp) ----

    // The scene area left free by the menu, toolbar, status bar and panels, in
    // logical window coordinates; recorded every frame by draw_panels(). A gesture
    // or hover query only starts inside it.
    bool point_in_scene(int x, int y) const;

    void set_select_mode(SelectMode mode);

    // Runs work queued by input callbacks: they cannot touch GPU resources because
    // the graphics context is not guaranteed current during event dispatch. Called
    // first in draw_gui(), when it is.
    void process_pending_work();

    // Applies a finished click/region gesture to the current model.
    void perform_selection(int x, int y, SelectAction action);

    void cancel_selection_gesture();

    void select_region(SelectAction action);

    void select_all_primitives();

    void clear_selection();

    void invert_selection();

    void confirm_delete_selected();

    void delete_selected_primitives();

    // Number of selected faces or points of the current model (cached; see
    // invalidate_selection_count()).
    std::size_t selection_count();

    void invalidate_selection_count() { selection_count_dirty_ = true; }

    // Shows the selection as v:select/f:select scalar coloring, remembering the
    // drawable's previous appearance the first time. Selection changes refresh only
    // the texture coordinates; a topology change needs a renderer update first.
    void refresh_selection_display(easy3d::Model *model);

    // Refreshes the selection display, or restores the saved appearance when the
    // selection became empty.
    void finish_selection_change(easy3d::Model *model);

    // Restores the appearance saved by refresh_selection_display(), if any.
    void restore_selection_style(easy3d::Model *model);

    // Drops saved styles and hover references to a model that is about to be deleted.
    void forget_model(easy3d::Model *model);

    // Hover readout and primitive highlight (throttled; see update_hover()).
    void update_hover(int x, int y);

    void clear_hover();

    // Appends the rubber-band/lasso hint geometry to the overlay commands; the
    // caller inserts them before the ImGui geometry so hints stay under the UI.
    void append_selection_overlay(std::vector<easy3d::graphics::DrawCommand> &commands) const;

    // Appends the current model's manipulated-frame box while Alt is held (the
    // manipulation gesture) or while its transform is not the identity.
    void append_manipulator_frame(std::vector<easy3d::graphics::DrawCommand> &commands) const;

private:
    std::unique_ptr<ImGuiLayer> imgui_;

    // Loaded in the constructor, written back in the destructor. Mutable because
    // save() is a const Viewer override that still records the saved file.
    mutable AppSettings settings_;

    float menu_height_ = 0.0f; // height of the main menu bar this frame

    bool show_rendering_panel_ = true;
    bool show_models_panel_ = true;
    bool show_log_panel_ = true;
    bool show_about_ = false;

    // Properties window: the model it edits, whether internal bookkeeping properties
    // are listed, and the pending rename (property, location, edited name).
    bool show_properties_window_ = false;
    bool focus_properties_window_ = false;
    bool show_internal_properties_ = false;
    easy3d::Model *properties_model_ = nullptr;
    bool open_rename_popup_ = false;
    std::string rename_property_;
    easy3d::State::Location rename_location_ = easy3d::State::VERTEX;
    std::string rename_error_;
    char rename_buffer_[128] = {};

    bool show_snapshot_dialog_ = false;
    SnapshotParams snapshot_params_;

    bool show_animation_window_ = false;
    int camera_keyframe_index_ = -1; // keyframe last jumped to, for Prev/Next

    float left_width_ = 250.0f;
    float right_width_ = 300.0f;
    float log_height_ = 170.0f;

    std::mutex log_mutex_;
    std::deque<LogLine> log_;
    std::size_t log_generation_ = 0;     // incremented on every appended line
    std::size_t last_seen_generation_ = 0; // UI-thread view, for auto-scrolling
    bool log_auto_scroll_ = true;
    static constexpr std::size_t max_log_lines_ = 2000;

    easy3d::Model *pending_delete_ = nullptr; // deferred so the Models panel never deletes mid-iteration
    bool mouse_captured_ = false;             // ImGui owned the press; keep the whole gesture away from the scene
    bool manipulating_model_ = false;         // an Alt+drag started this gesture on the current model

    int scene_x_ = 0, scene_y_ = 0, scene_w_ = 0, scene_h_ = 0; // logical, updated every frame

    SelectMode select_mode_ = SelectMode::OFF;
    bool selecting_ = false;            // a selection gesture is in progress
    SelectAction select_action_ = SelectAction::REPLACE; // from the press modifiers
    easy3d::vec2 select_start_ = easy3d::vec2(0); // logical window coordinates
    easy3d::vec2 select_end_ = easy3d::vec2(0);
    std::vector<easy3d::vec2> lasso_; // logical window coordinates

    // Deferred work: input callbacks record requests, draw_gui() executes them.
    bool pending_mode_ = false;
    SelectMode pending_select_mode_ = SelectMode::OFF;
    bool pending_pick_ = false;
    bool pending_region_ = false;
    int pending_x_ = 0, pending_y_ = 0;
    SelectAction pending_action_ = SelectAction::REPLACE;
    SelectCommand pending_command_ = SelectCommand::NONE;
    bool pending_hover_ = false;
    int hover_x_ = 0, hover_y_ = 0;

    // Pickers borrow the camera and own backend pick resources; created on first use.
    std::unique_ptr<easy3d::PointCloudPicker> point_picker_;
    std::unique_ptr<easy3d::SurfaceMeshPicker> mesh_picker_;

    std::map<easy3d::Model *, SelectionStyle> selection_styles_;

    // Keyed by model, then drawable name: a renderer update may recreate drawables
    // while keeping their names.
    std::map<easy3d::Model *, std::map<std::string, ScalarFieldStyle>> scalar_styles_;
    const easy3d::Model *counted_model_ = nullptr;
    std::size_t selected_count_ = 0;
    bool selection_count_dirty_ = true;

    bool show_pick_readout_ = true;
    easy3d::Model *hover_model_ = nullptr;       // clears the hover when its model is deleted
    std::weak_ptr<easy3d::Drawable> hover_drawable_;
    std::pair<int, int> hover_range_ = {-1, -1}; // highlighted primitives
    std::string hover_text_;
    std::chrono::steady_clock::time_point hover_stamp_;

    ProcessOp process_op_ = ProcessOp::NONE;
    ProcessParams process_params_;
    std::unique_ptr<ProcessJob> process_job_;

    // Created in init() (they allocate GPU resources), dropped in the destructor
    // before the graphics device dies with the viewer.
    std::unique_ptr<easy3d::AmbientOcclusion> ao_;
    std::unique_ptr<easy3d::EyeDomeLighting> edl_;
    std::unique_ptr<easy3d::SoftShadow> shadow_;
    std::unique_ptr<easy3d::Transparency> transparency_;

    // The recording that owns the screen and blocks scene input until it finishes
    // or is cancelled.
#ifdef HAS_FFMPEG
    std::unique_ptr<VideoRecording> video_recording_;
    int video_bitrate_mbps_ = 8;
#endif

    static const char *process_title(ProcessOp op);
};


#endif // EASY3D_MAPPLE_VIEWER_H
