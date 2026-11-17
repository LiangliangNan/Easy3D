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
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/fileio/translator.h>
#include <easy3d/renderer/camera_path_builder.h>
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
    class KeyFrameInterpolator;
    class LinesDrawable;
    class Model;
    class PointCloudPicker;
    class SoftShadow;
    class TrianglesDrawable;
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
        // Distance-threshold simplification: uniform (minimum point spacing) or grid
        // (one representative per cell). Querying the average spacing fills the
        // read-only display so the threshold can be chosen relative to it.
        bool point_simplify_by_distance = false;
        bool point_simplify_uniform = true;
        float point_simplify_distance = 0.01f;
        float point_average_spacing = -1.0f;

        int poisson_depth = 8;
        float poisson_samples = 1.0f;
        bool poisson_trim = false;
        float poisson_trim_value = 6.0f;
        float poisson_island_ratio = 0.001f;

        bool ransac_plane = true;
        bool ransac_sphere = false;
        bool ransac_cylinder = false;
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
        int smooth_iterations = 5;
        int simplify_target = 0;
        int remesh_scheme = 0;          // 0: uniform, 1: adaptive
        bool remesh_use_features = true;
        int remesh_dihedral = 60;
        float remesh_edge_factor = 1.0f;
        int hole_max_size = 50;
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

    // A highlight range configured in the Rendering panel. The indices are rendered
    // primitives (points, lines, or triangles, i.e., polygons are triangulated).
    struct HighlightRange {
        bool enabled = false;
        int first = 0;
        int last = 0;
    };

    // How a textured drawable samples its image: the file chosen in the Rendering
    // panel and, optionally, a stripe count that quantizes the image into bands.
    struct TextureStyle {
        std::string file;      // seeded from the drawable's texture when that is a file
        bool discrete = false;
        int stripes = 16;
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

    // Transparency method: 0 average color blending, 1 dual depth peeling, 2 per-pixel
    // linked lists. Rebuilds the effect when it is running and records the choice in
    // setting::effect_transparency_method.
    void set_transparency_method(int method);
    int transparency_method() const { return transparency_method_; }
    // Whether \p method can run on this backend: the linked lists need Vulkan or
    // OpenGL 4.3+ (macOS OpenGL is limited to 4.1).
    static bool transparency_method_supported(int method);

    bool ao_enabled() const { return ao_ != nullptr; }

    bool edl_enabled() const { return edl_ != nullptr; }

    bool shadow_enabled() const { return shadow_ != nullptr; }

    bool transparency_enabled() const { return transparency_ != nullptr; }

    // Pushes the effect parameters in settings_ into the live effects and redraws.
    void apply_effect_parameters();

    // ui/menu.cpp
    void draw_menu_bar();

    // One popup-menu row: the named icon in the icon column, the label, and the shortcut
    // right-aligned. MenuItem() cannot show an image, so the row is drawn over a hidden
    // Selectable (which still owns hover, activation and popup closing). Falls back to
    // MenuItem() when the icon is unavailable.
    bool icon_menu_item(const char *icon_name, const char *label, const char *shortcut = nullptr,
                        bool enabled = true);

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

    // The former body of apply_process(): runs the command. May run on a worker
    // thread, so it must not touch GPU resources or add models directly.
    void run_process_algorithm(easy3d::Model *model);

    void start_process_job();

    // Joins a finished job and adopts its result model on the main thread.
    void poll_process_job();

    void finalize_process(easy3d::Model *model, std::chrono::steady_clock::time_point start);

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

    // Steps the keyframe interpolation one sample when its period has elapsed; called
    // from pre_draw() while playback runs, since the base Viewer no longer owns one.
    void advance_camera_path();

    // Stops playback and moves the camera to a keyframe.
    void goto_camera_keyframe(int index);

    // Shows/hides the camera path and keyframe cameras, widening the scene radius
    // while visible so the whole path stays in the depth range.
    void set_camera_path_visible(bool visible);

    void import_camera_path();

    void export_camera_path();

    // The one-shot tool the app has armed; Off means no tool owns the next gesture.
    enum class PathTool {
        Off,         // no path tool is armed
        GroundWalk,  // the next drag on the ground becomes a walking route
        Orbit,       // the next click picks the orbit axis
        Flight       // a ground drag traces the camera track, then a click or drag sets the aim
    };

    // One-shot path tools. Arming makes the next gesture in the scene belong to the
    // tool - a drag traces a walking route along the ground, a click picks the orbit
    // axis, and a flight traces its camera track then its aim - and the tool disarms
    // itself once its gestures are applied. Esc or the window's Cancel button disarms
    // it immediately.
    void arm_camera_path_tool(PathTool tool);
    void cancel_camera_path_tool();
    void cancel_path_gesture();

    // Turns the traced stroke into walking keyframes appended to the path. The
    // stroke is split into segments where a drag sample missed the visible surface.
    void commit_ground_route();

    // Turns the traced stroke into flight stations (lifted by the flight height) and
    // waits for the aim gesture; a click aims every station at one target, a drag
    // traces an aim stroke that is sampled per station.
    void commit_flight_track();
    void commit_flight_aim();
    void clear_flight_gesture();

    // Applies the orbit gesture: the axis passes through the surface under \p from
    // and runs along the dragged screen segment; a click (no drag) keeps the
    // turntable default, the view's up axis through that point. The orbit replaces
    // the path or is appended to it, as the Orbit tool is set.
    void apply_orbit_gesture(const easy3d::vec2 &from, const easy3d::vec2 &to);

    // The path as it was before the last path tool was armed or keyframe edit, for
    // Ctrl+Z. One level: restoring it consumes the snapshot.
    struct CameraPathSnapshot {
        std::vector<easy3d::Frame> frames;
        std::vector<float> times;
        int keyframe_index = -1;
        bool valid = false;
    };
    void snapshot_camera_path();
    // Restores the snapshot taken before the last tool or edit; false without one.
    bool undo_camera_path();

    // Keyframe handles: with editing on, the selected keyframe shows a gizmo sized to
    // its camera glyph. A press on the glyph drags it in the view plane through its
    // position (the plane through its up with Shift), an axis arrow drags it along
    // that axis, and a ring turns its pose around that axis. Esc restores the pose.
    enum class KeyframeHandle {
        none, free,
        move_x, move_y, move_z,
        turn_x, turn_y, turn_z
    };
    // The selected keyframe whose gizmo is shown and edited, or -1.
    int selected_keyframe() const;
    // The gizmo's world axes, arrow length and ring radius: the keyframe's own axes,
    // sized relative to camera_marker_width() so it hugs the drawn glyph.
    void keyframe_handle_axes(easy3d::vec3 axes[3], float &arrow_length, float &ring_radius) const;
    // The handle under the cursor (nearest screen element within a few pixels); none
    // when no keyframe is selected or the element is seen end-on.
    KeyframeHandle keyframe_handle_at(int x, int y) const;
    bool begin_keyframe_drag(int x, int y, bool upright_plane);
    void drag_keyframe(int x, int y);
    void end_keyframe_drag();
    void cancel_keyframe_drag();

    // Inserts a keyframe after the selected one: midway to the next, with the pose and
    // time interpolated the same way; past the last keyframe the last segment is
    // extended so the new glyph does not sit on top of an existing one.
    void insert_camera_keyframe();
    // Deletes the selected keyframe.
    void delete_camera_keyframe();
    // Replaces every keyframe with \p frames and their \p times. The interpolator only
    // appends, so the edits rebuild the path through this.
    bool rebuild_camera_path(const std::vector<easy3d::Frame> &frames, const std::vector<float> &times);

    // The visible surface under (x, y): the ground-contact point of a route sample
    // and the walker's up direction there (the picked face's normal, turned to the
    // side facing the camera; the view's up itself when the hit has no face). False
    // when the ray misses every visible primitive.
    bool route_point_at(int x, int y, easy3d::vec3 &point, easy3d::vec3 &up) const;

    // Width, in world units, of the keyframe camera glyphs; the drawn glyphs and their
    // rebuilt geometry use the same width.
    float camera_marker_width() const;

    // Rebuilds the path and the keyframe camera glyphs in the scene: one camera glyph
    // per keyframe and the interpolated polyline. They are ordinary drawables, so the
    // scene occludes them and the pivot gesture lands on the glyphs; both are hidden
    // while the path plays and removed when it is hidden.
    void update_camera_path_guides();

    // Removes the guide drawables so the scene bounds no longer include them; the
    // caller restores the bounds afterwards.
    void release_camera_path_guides();

    // Draws the path gesture preview: the walking stroke being traced, or the orbit
    // axis being dragged, in the scene's overlay layer.
    void append_path_stroke_overlay(std::vector<easy3d::graphics::DrawCommand> &commands) const;

    // Draws the selected keyframe's gizmo (three axis arrows and three turn rings)
    // in the overlay layer; the grabbed or hovered handle is drawn prominent and the
    // others are dimmed.
    void append_keyframe_gizmo_overlay(std::vector<easy3d::graphics::DrawCommand> &commands) const;

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

    // Writes/restores the window geometry, background color, projection, camera and
    // snapshot settings to/from a .view file (JSON). A parameter missing from the
    // file keeps its value.
    void save_view_state();
    void save_view_state(const std::string& file_path);

    void restore_view_state();

    // Adds a file chosen from File > Open Recent and fits the camera to it.
    void open_recent_file(const std::string &file);

    // File > Loading option: configures the process-wide Translator consulted by the
    // file loaders, and logs the new mode.
    void set_load_translation(easy3d::Translator::Status status);

    // ui/panels.cpp
    void draw_toolbar(float x, float y, float w, float h);

    // One toolbar button: the named icon from resources/icons with `label` as its tooltip,
    // or a plain text button when the icon is unavailable.
    bool toolbar_button(const char *icon_name, const char *label);

    void draw_model_info(easy3d::Model *model);

    void draw_panels(float x, float y, float w, float h);

    void draw_rendering_panel(float x, float y, float w, float h);

    // Color scheme combo of one drawable (uniform color, a color property, texture
    // coordinates, or a scalar field) and, for scalar fields, the colormap controls.
    void draw_color_scheme(easy3d::Model *model, easy3d::Drawable *drawable);

    // The texture of a scalar field style, from the colormap list in ui/panels.cpp.
    easy3d::Texture *scalar_texture(const ScalarFieldStyle &style) const;

    // Texture controls of a textured drawable: the image, its optional stripes,
    // and the repeat factors.
    void draw_texture_controls(easy3d::Model *model, easy3d::Drawable *drawable);

    // Vector-field block of the editor: the vec3 fields the drawable can show,
    // and the scale of the shared "vector - <field>" lines drawable.
    void draw_vector_field_controls(easy3d::Model *model, easy3d::Drawable *drawable);

    // Shows the field's lines drawable, creating it with a buffer update function
    // on first use. The location is where the drawable samples the field.
    void show_vector_field(easy3d::Model *model, const std::string &field,
                           easy3d::State::Location location);

    // Re-applies a scalar field style to the named property. The property must be
    // passed explicitly: the drawable's own name is reset when it switches to
    // uniform coloring, so it cannot be used to return to the previous scalar field.
    void apply_scalar_style(easy3d::Drawable *drawable, easy3d::State::Location location,
                            const std::string &property, const ScalarFieldStyle &style);

    void draw_models_panel(float x, float y, float w, float h);

    // Whether the row is part of the Models panel's selection.
    bool model_row_selected(const easy3d::Model *model) const;

    // Applies a click on a row: plain click selects only that row, Ctrl/Cmd toggles
    // it, Shift extends the selection from the anchor. Makes the row the active model,
    // refreshes the selection view, and fits the camera when auto focus is on.
    void click_model_row(easy3d::Model *model);

    // The rows a batch command acts on, in scene order: the selection, or the active
    // model when nothing is selected.
    std::vector<easy3d::Model *> selected_models() const;

    // Drops selected, renamed or anchored models that are no longer in the scene.
    void prune_model_selection();

    // Replaces the selection with its complement in scene order.
    void invert_model_selection();

    // Models panel commands (context menu and buttons). Each logs its result.
    void duplicate_model(easy3d::Model *model);
    void merge_selected_models();
    void decompose_selected_models();
    // One confirmation for the whole selection; the deletion itself is deferred to
    // the end of the panel so the list is never modified mid-iteration.
    void delete_selected_models();
    // Copies the active model's drawable styles (state, point/line/triangle extras and
    // the panel's scalar/texture records) onto same-named drawables of every other
    // model. Visibility and topology-dependent records stay per model.
    void apply_style_to_all_models();
    // One drawable kind of apply_style_to_all_models(): pairs by name.
    void propagate_drawable_style(easy3d::Model *source, easy3d::Model *target,
                                  const std::vector<easy3d::Drawable *> &from_drawables,
                                  const std::vector<easy3d::Drawable *> &to_drawables);
    void begin_rename_model(easy3d::Model *model);
    void commit_rename_model();
    // Selected only: the active model visible and every other model hidden; turning it
    // off shows every model again, as the Qt panel did.
    void set_models_selected_only(bool on);
    void set_models_auto_focus(bool on);

    void draw_log_panel(float x, float y, float w, float h);

    void draw_status_bar(float x, float y, float w, float h);

    void draw_about_window();

    // A 4 px drag handle between the scene and a panel. dragging along the axis
    // adjusts *value by sign * mouse_delta.
    void draw_splitter(const char *id, float x, float y, float w, float h,
                       bool resize_width, float *value, float sign,
                       float min_value, float max_value);

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

    // Tints the elements marked in v:select/f:select over the drawable's own
    // coloring, and drops the tint when the selection is empty.
    void refresh_selection_display(easy3d::Model *model);

    // Rebuilds the selection drawable's buffers from the model. A topology change
    // (deleted vertices or faces) leaves the drawable with the geometry from before
    // it, while the highlight mask is sized per rendered primitive, so the mask must
    // be applied only after the buffers reflect the new model.
    void refresh_selection_after_model_change(easy3d::Model *model);

    // Refreshes the selection display after a selection change.
    void finish_selection_change(easy3d::Model *model);

    // Drops saved styles and hover references to a model that is about to be deleted.
    void forget_model(easy3d::Model *model);

    // Hover readout and the hovered face's edge outline (throttled; see update_hover()).
    void update_hover(int x, int y);

    void clear_hover();

    // ---- distance measurement (mapple_viewer.cpp) ----

    // Arms the one-shot two-click distance tool; pressing it again (or Esc) clears the
    // measurement and disarms. The first click places a marker, the second completes
    // the segment, reports the distance in the status bar and disarms the tool.
    void arm_measure();

    void measure_click(int x, int y);

    // Hides the guides, disarms the tool and drops the status-bar readout.
    void clear_measurement();

    // Appends the rubber-band/lasso hint geometry to the overlay commands; the
    // caller inserts them before the ImGui geometry so hints stay under the UI.
    void append_selection_overlay(std::vector<easy3d::graphics::DrawCommand> &commands) const;

    // Appends the hovered face's fill: a depth-tested world-space triangle overlay,
    // so the cue never touches the drawable's highlight mask.
    void append_hover_overlay(std::vector<easy3d::graphics::DrawCommand> &commands) const;

    // Appends the distance annotation: the endpoint markers and the segment between
    // them. It is a screen-space overlay because the chord between two surface points
    // runs through the model and scene geometry would be occluded.
    void append_measure_overlay(std::vector<easy3d::graphics::DrawCommand> &commands) const;

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
    // A restored view state carries the sample count its figure was rendered with;
    // opening the dialog must not re-seed it from the window.
    bool snapshot_samples_restored_ = false;

    bool show_animation_window_ = false;
    int camera_keyframe_index_ = -1; // keyframe last jumped to, for Prev/Next
    // The camera path (keyframes, playback state and drawing) belongs to the app: the
    // base Viewer no longer owns a KeyFrameInterpolator.
    std::unique_ptr<easy3d::KeyFrameInterpolator> kfi_;
    bool show_camera_path_ = false;
    std::chrono::steady_clock::time_point next_camera_path_frame_;
    // The keyframe camera glyphs and the interpolated path: standalone drawables in the
    // scene. The stored poses and width are the ones the glyph buffers were built with.
    std::shared_ptr<easy3d::TrianglesDrawable> camera_markers_;
    std::shared_ptr<easy3d::LinesDrawable> camera_path_line_;
    std::vector<easy3d::Frame> camera_markers_frames_;
    float camera_markers_width_ = 0;
    std::size_t camera_markers_faces_per_glyph_ = 0; // maps a picked triangle to its keyframe
    // Keyframe handle editing: the gizmo can be grabbed while this is on. The drag
    // plane (a point and a normal) is fixed at the press so the pose follows the
    // cursor; an axis drag keeps a line parameter, a ring drag an accumulated angle.
    bool edit_keyframes_ = false;
    int dragged_keyframe_ = -1;
    KeyframeHandle keyframe_handle_ = KeyframeHandle::none;
    KeyframeHandle hovered_keyframe_handle_ = KeyframeHandle::none;
    easy3d::vec3 keyframe_drag_plane_point_;
    easy3d::vec3 keyframe_drag_plane_normal_;
    easy3d::vec3 keyframe_drag_start_position_;
    easy3d::quat keyframe_drag_start_orientation_;
    easy3d::vec3 keyframe_drag_axis_;          // the grabbed arrow's or ring's axis
    float keyframe_drag_start_scalar_ = 0.0f;  // arrow grab: line parameter at the press
    float keyframe_drag_angle_ = 0.0f;         // ring grab: accumulated angle
    float keyframe_drag_last_angle_ = 0.0f;    // ring grab: previous cursor angle
    bool keyframe_drag_moved_ = false;
    // Walking, orbit and flight keyframe creation (created in init()).
    std::unique_ptr<easy3d::CameraPathBuilder> camera_path_builder_;
    // Which one-shot path tool owns the next gesture.
    PathTool path_tool_ = PathTool::Off;

    // The gesture owned by the armed one-shot path tool. A route stroke keeps the
    // accepted screen trail for the live preview and the picked surface samples,
    // split into segments where the drag left the visible surface.
    bool tracing_route_ = false;
    // A route sample: the surface point under the cursor and the walker's up there.
    struct RouteSample {
        easy3d::vec3 point;
        easy3d::vec3 up;
    };
    std::vector<easy3d::vec2> route_stroke_screen_;          // accepted drag trail, logical window coordinates
    std::vector<RouteSample> route_segment_;                 // picked samples of the current segment
    std::vector<std::vector<RouteSample>> route_segments_;   // segments completed by a missed pick

    // Flight: which phase the stroke belongs to, and the stations awaiting the aim.
    bool flight_tracking_ = false;
    bool flight_aiming_ = false;
    bool flight_aim_ready_ = false;
    std::vector<easy3d::vec3> flight_positions_;
    // The orbit gesture: a drag across the scene sets the axis (previewed as the
    // dragged segment); the release applies it.
    bool orbit_dragging_ = false;
    easy3d::vec2 orbit_drag_from_;
    easy3d::vec2 orbit_drag_to_;
    // The Orbit tool clears the path before generating instead of appending.
    bool orbit_replaces_path_ = true;
    CameraPathSnapshot camera_path_snapshot_;

    float left_width_ = 250.0f;
    float right_width_ = 300.0f;
    float log_height_ = 170.0f;

    std::mutex log_mutex_;
    std::deque<LogLine> log_;
    std::size_t log_generation_ = 0;     // incremented on every appended line
    std::size_t last_seen_generation_ = 0; // UI-thread view, for auto-scrolling
    bool log_auto_scroll_ = true;
    static constexpr std::size_t max_log_lines_ = 2000;

    // Deferred so the Models panel never modifies the scene while iterating its list.
    std::vector<easy3d::Model *> pending_deletes_;
    std::set<easy3d::Model *> model_selection_;
    easy3d::Model *model_selection_anchor_ = nullptr; // Shift-click range start
    easy3d::Model *renaming_model_ = nullptr;         // the row that shows an edit box
    char model_rename_buffer_[256] = {};
    bool rename_focus_ = false;          // focus the edit box on the next frame
    bool models_auto_focus_ = false;     // fit the camera when the active model changes
    bool models_selected_only_ = false;  // show only the active model
    bool mouse_captured_ = false;        // ImGui owned the press; keep the whole gesture away from the scene
    bool manipulating_model_ = false;    // an Alt+drag started this gesture on the current model

    int scene_x_ = 0, scene_y_ = 0, scene_w_ = 0, scene_h_ = 0; // logical, updated every frame

    SelectMode select_mode_ = SelectMode::OFF;
    bool selecting_ = false;            // a selection gesture is in progress
    SelectAction select_action_ = SelectAction::REPLACE; // from the press modifiers
    easy3d::vec2 select_start_ = easy3d::vec2(0); // logical window coordinates
    easy3d::vec2 select_end_ = easy3d::vec2(0);
    std::vector<easy3d::vec2> lasso_; // logical window coordinates

    // Distance measurement: the armed one-shot tool and the picked points
    // (0: none, 1: first placed, 2: complete). The annotation is drawn as an overlay.
    bool measure_armed_ = false;
    bool pending_measure_pick_ = false;
    int measure_pick_x_ = 0, measure_pick_y_ = 0;
    int measure_point_count_ = 0;
    easy3d::vec3 measure_start_ = easy3d::vec3(0);
    easy3d::vec3 measure_end_ = easy3d::vec3(0);
    float measure_distance_ = 0.0f;

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

    // Keyed by model, then drawable name: a renderer update may recreate drawables
    // while keeping their names.
    std::map<easy3d::Model *, std::map<std::string, ScalarFieldStyle>> scalar_styles_;

    // Highlight ranges set in the Rendering panel, keyed like the scalar styles.
    std::map<easy3d::Model *, std::map<std::string, HighlightRange>> highlight_ranges_;

    // Texture settings and vector-field selection, keyed like the scalar styles.
    // The vector-field scale is per field because the "vector - <field>" lines
    // drawable is shared by every drawable that shows the same field.
    std::map<easy3d::Model *, std::map<std::string, TextureStyle>> texture_styles_;
    std::map<easy3d::Model *, std::map<std::string, std::string>> vector_fields_;      // drawable -> field
    std::map<easy3d::Model *, std::map<std::string, float>> vector_field_scales_;      // field -> scale

    // The drawable whose editor the Rendering panel shows, by name for the same
    // reason: it survives a renderer update that recreates the drawables.
    std::string active_drawable_name_;
    const easy3d::Model *counted_model_ = nullptr;
    std::size_t selected_count_ = 0;
    bool selection_count_dirty_ = true;

    bool show_pick_readout_ = true;
    easy3d::Model *hover_model_ = nullptr;       // clears the hover when its model is deleted
    std::vector<easy3d::vec3> hover_face_;       // world-space triangles of the hovered face
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
    int transparency_method_ = 1;

    // Builds transparency_ for transparency_method_ and the current parameters.
    void create_transparency();

    // The recording that owns the screen and blocks scene input until it finishes
    // or is cancelled.
#ifdef HAS_FFMPEG
    std::unique_ptr<VideoRecording> video_recording_;
    int video_bitrate_mbps_ = 8;
#endif

    static const char *process_title(ProcessOp op);
};


#endif // EASY3D_MAPPLE_VIEWER_H
