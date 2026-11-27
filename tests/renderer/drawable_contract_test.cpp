// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/drawable_renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/setting.h>
#include <easy3d/core/point_cloud.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace easy3d;
using namespace easy3d::graphics;

namespace {

    const Drawable::BufferData &data(const Drawable &drawable) {
        return drawable.buffer_data();
    }

    void require(bool value, const char *message) {
        if (!value)
            throw std::runtime_error(message);
    }

    template<class Function>
    void rejects(Function function, const char *message = "invalid drawable input was accepted") {
        bool rejected = false;
        try {
            function();
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        require(rejected, message);
    }

    template<class Function>
    void overflows(Function function, const char *message = "revision exhaustion did not reject the update") {
        bool rejected = false;
        try {
            function();
        } catch (const std::overflow_error &) {
            rejected = true;
        }
        require(rejected, message);
    }

    class TestTriangles final : public TrianglesDrawable {
    public:
        using TrianglesDrawable::TrianglesDrawable;
        using Drawable::clear;
    };

    class RealizingTriangles final : public TrianglesDrawable {
    public:
        using TrianglesDrawable::TrianglesDrawable;

        bool colors = true;
        bool indices = true;

    protected:
        void update_buffers_internal() override {
            ++calls;
            update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
            if (colors)
                update_color_buffer({vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)});
            else
                update_color_buffer({});
            if (indices)
                update_element_buffer({0, 1, 2});
            else
                disable_element_buffer();
        }

    public:
        int calls = 0;
    };

    void test_geometry_updates() {
        TestTriangles triangles("triangles");
        const auto *snapshot = &data(triangles);
        require(snapshot->vertices.empty() && snapshot->revision == 0,
                "new drawable does not have empty portable geometry");
        require(&triangles.buffer_data() == snapshot,
                "compatibility buffer_data overload changed snapshot identity");

        triangles.update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        triangles.update_element_buffer({0, 1, 2});
        triangles.update_normal_buffer({vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1)});
        triangles.update_color_buffer({vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)});
        triangles.update_texcoord_buffer({vec2(0, 0), vec2(1, 0), vec2(0, 1)});
        triangles.set_source_faces({7});
        const auto revision = data(triangles).revision;
        require(&data(triangles) == snapshot, "buffer snapshot identity changed");
        require(data(triangles).source_faces == std::vector<int>{7},
                "triangle provenance was not retained");
        require(triangles.num_vertices() == 3 && triangles.num_indices() == 3,
                "portable upload did not update element counts");
        require(triangles.bounding_box().max_point().x == 1 && triangles.bounding_box().max_point().y == 1,
                "standalone upload did not update its bounding box");
        const auto uploaded = data(triangles);
        triangles.update();
        require(data(triangles).vertices == uploaded.vertices &&
                        data(triangles).colors == uploaded.colors &&
                        data(triangles).normals == uploaded.normals &&
                        data(triangles).texcoords == uploaded.texcoords &&
                        data(triangles).indices == uploaded.indices &&
                        data(triangles).source_faces == uploaded.source_faces &&
                        data(triangles).revision == uploaded.revision &&
                        triangles.bounding_box().max_point().x == 1,
                "update() erased or invalidated standalone client geometry");

        triangles.update_vertex_buffer({vec3(2, 2, 2), vec3(4, 2, 2), vec3(2, 4, 2),
                                        vec3(2, 2, 4), vec3(4, 2, 4), vec3(2, 4, 4)}, true);
        require(data(triangles).revision > revision &&
                        data(triangles).source_faces.empty() &&
                        triangles.num_vertices() == 6,
                "resizing vertices did not invalidate provenance and advance revision");
        triangles.update_element_buffer({0, 1, 2, 3, 4, 5});
        rejects([&] { triangles.set_source_faces({1}); }, "wrong provenance count was accepted");
        rejects([&] { triangles.set_source_faces({-2, -2}); }, "invalid provenance ID was accepted");

        triangles.set_source_faces({3, 4});
        triangles.disable_element_buffer();
        require(data(triangles).indices.empty() &&
                        data(triangles).source_faces.empty() && triangles.num_indices() == 0,
                "disabling the element buffer did not clear portable indices and provenance");
        triangles.update_vertex_buffer({});
        require(data(triangles).vertices.empty() && triangles.num_vertices() == 0,
                "empty resizing upload retained old vertices");
        triangles.update_vertex_buffer({vec3(1, 2, 3)});
        triangles.clear();
        require(data(triangles).vertices.empty(), "clear retained portable geometry");
    }

    void test_callback_realization_and_detachment() {
        auto cloud = std::make_shared<PointCloud>();
        cloud->add_vertex(vec3(1, 2, 3));
        cloud->add_vertex(vec3(4, 5, 6));
        PointsDrawable points("points", cloud.get());
        require(data(points).vertices.size() == 2,
                "model geometry was not lazily extracted into portable buffers");
        const auto revision = data(points).revision;
        require(data(points).revision == revision,
                "reading unchanged portable buffers advanced revision");
        points.set_model(nullptr);
        require(data(points).vertices.empty() &&
                        data(points).revision > revision,
                "detaching a drawable retained model geometry");

        TestTriangles callback("callback");
        int calls = 0;
        callback.set_update_func([&calls](Model *, Drawable *target) {
            ++calls;
            target->update_vertex_buffer({vec3(float(calls), 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        });
        callback.update();
        require(data(callback).vertices[0].x == 1 && calls == 1,
                "pending callback was not realized exactly once");
        require(data(callback).vertices[0].x == 1 && calls == 1,
                "unchanged callback geometry was extracted twice");
        callback.update();
        require(data(callback).vertices[0].x == 2 && calls == 2,
                "repeated callback update did not replace portable geometry");
        callback.set_update_func({});
        callback.clear();
        require(data(callback).vertices.empty(), "clear retained callback geometry");
    }

    void test_explicit_attributes_survive_realization() {
        auto cloud = std::make_shared<PointCloud>();
        cloud->add_vertex(vec3(9, 9, 9));
        cloud->add_vertex(vec3(8, 8, 8));
        PointsDrawable points("points", cloud.get());
        points.update_normal_buffer({vec3(0, 0, 1), vec3(0, 0, 1)});
        require(data(points).vertices.size() == 2 &&
                        data(points).normals == std::vector<vec3>({vec3(0, 0, 1), vec3(0, 0, 1)}),
                "model extraction discarded explicitly staged normals");
        points.update_vertex_buffer({vec3(1, 2, 3)});
        require(data(points).vertices[0] == vec3(1, 2, 3),
                "an explicit vertex upload did not replace model geometry");

        TestTriangles callback("callback");
        int calls = 0;
        callback.set_update_func([&calls](Model *, Drawable *target) {
            ++calls;
            target->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        });
        callback.update_element_buffer({0, 1, 2});
        const auto &realized = data(callback);
        require(calls == 1 && realized.vertices.size() == 3 && realized.indices.size() == 3,
                "an explicit index upload suppressed or was overwritten by callback geometry");
        callback.update();
        const auto &refreshed = data(callback);
        require(calls == 2 && refreshed.vertices.size() == 3 && refreshed.indices.size() == 3,
                ("callback refresh state: calls=" + std::to_string(calls) + ", vertices=" +
                 std::to_string(refreshed.vertices.size()) + ", indices=" + std::to_string(refreshed.indices.size())).c_str());
    }

    void test_realized_index_mode_does_not_override_later_updates() {
        TestTriangles callback("index mode callback");
        int calls = 0;
        callback.set_update_func([&calls](Model *, Drawable *target) {
            ++calls;
            target->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
            if (calls == 2)
                target->disable_element_buffer();
            else
                target->update_element_buffer({0, 1, 2});
        });
        callback.update();
        require(data(callback).indices.size() == 3, "indexed callback geometry was not realized");
        callback.update();
        require(data(callback).indices.empty(), "callback could not switch to non-indexed geometry");
        callback.update();
        require(data(callback).indices == std::vector<unsigned int>{0, 1, 2},
                "realized non-indexed geometry overrode a later indexed callback");
    }

    void test_realization_exception_recovery() {
        TestTriangles callback("throwing callback");
        int calls = 0;
        callback.set_update_func([&calls](Model *, Drawable *target) {
            if (++calls == 1) {
                target->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
                target->update_element_buffer({0, 1, 2});
                target->update_normal_buffer({vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1)});
                throw std::runtime_error("realization failed");
            }
            target->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        });

        bool rejected = false;
        try {
            static_cast<void>(data(callback));
        } catch (const std::runtime_error& error) {
            rejected = std::string(error.what()) == "realization failed";
        }
        require(rejected, "realization exception did not reach the caller");

        callback.update_normal_buffer({vec3(0, 1, 0), vec3(0, 1, 0), vec3(0, 1, 0)});
        require(data(callback).vertices.size() == 3 && calls == 2,
                "drawable did not recover after a realization exception");
        require(data(callback).normals == std::vector<vec3>({vec3(0, 1, 0), vec3(0, 1, 0), vec3(0, 1, 0)}),
                "failed realization left stale callback state and discarded a client upload");
    }

    void test_standalone_virtual_realization() {
        RealizingTriangles triangles("virtual realization");
        require(data(triangles).vertices.size() == 3 && triangles.calls == 1,
                "standalone virtual realization was skipped or repeated");
        require(data(triangles).colors.size() == 3 && data(triangles).indices.size() == 3,
                "generated attributes became client-owned or were lost");

        triangles.colors = false;
        triangles.indices = false;
        triangles.update();
        const auto &refreshed = data(triangles);
        require(triangles.calls == 2 && refreshed.colors.empty() && refreshed.indices.empty(),
                "a virtual override could not remove generated attributes or indices");
        triangles.indices = true;
        triangles.update();
        const auto &indexed = data(triangles);
        require(triangles.calls == 3 && indexed.indices == std::vector<unsigned int>{0, 1, 2},
                "realized non-indexed geometry overrode a later virtual update");
    }

    void test_explicit_indices_clear_generated_provenance() {
        TestTriangles triangles("explicit indices");
        int calls = 0;
        triangles.set_update_func([&calls](Model *, Drawable *target) {
            ++calls;
            target->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
            target->update_element_buffer({0, 1, 2});
            target->set_source_faces({17});
            target->update_highlight_mask({.5f});
        });
        triangles.update_element_buffer({2, 1, 0, 0, 1, 2});
        require(data(triangles).indices == std::vector<unsigned int>{2, 1, 0, 0, 1, 2} &&
                        data(triangles).source_faces.empty() && triangles.highlight_mask().empty(),
                "explicit indices retained stale generated provenance or highlights");
        triangles.update();
        const auto &refreshed_indices = data(triangles);
        require(calls == 2 && refreshed_indices.source_faces.empty() && triangles.highlight_mask().empty(),
                "callback refresh reintroduced provenance for explicit client indices");
    }

    void test_first_use_provenance_and_highlights() {
        auto cloud = std::make_shared<PointCloud>();
        cloud->add_vertex(vec3(0, 0, 0));
        cloud->add_vertex(vec3(1, 0, 0));
        PointsDrawable points("pending highlights", cloud.get());
        points.update_highlight_mask({.5f, .25f});
        require(points.highlight_mask() == std::vector<float>{.5f, .25f} &&
                        data(points).vertices.size() == 2,
                "first-use highlight validation did not realize pending geometry");

        TestTriangles triangles("pending provenance");
        int calls = 0;
        triangles.set_update_func([&calls](Model *, Drawable *target) {
            ++calls;
            target->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        });
        triangles.set_source_faces({19});
        require(calls == 1 && data(triangles).source_faces == std::vector<int>{19},
                "first-use provenance validation did not realize pending geometry");
    }

    void test_const_realization_and_revision_exhaustion() {
        auto cloud = std::make_shared<PointCloud>();
        cloud->add_vertex(vec3(1, 2, 3));
        const PointsDrawable points("const points", cloud.get());
        require(data(points).vertices.size() == 1, "const model-backed drawable was not realized");

        TestTriangles triangles("revision exhaustion");
        triangles.update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        auto &snapshot = const_cast<Drawable::BufferData &>(data(triangles));
        snapshot.revision = std::numeric_limits<std::size_t>::max();
        triangles.set_update_func([](Model *, Drawable *target) {
            target->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        });
        triangles.update();
        bool rejected = false;
        try {
            static_cast<void>(data(triangles));
        } catch (const std::overflow_error &) {
            rejected = true;
        }
        require(rejected && snapshot.vertices.size() == 3 &&
                        snapshot.revision == std::numeric_limits<std::size_t>::max(),
                "revision exhaustion wrapped or discarded the previous geometry");

        TestTriangles updates("public update exhaustion");
        updates.update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        updates.update_color_buffer({vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)});
        updates.update_normal_buffer({vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1)});
        updates.update_texcoord_buffer({vec2(0, 0), vec2(1, 0), vec2(0, 1)});
        updates.update_element_buffer({0, 1, 2});
        updates.set_source_faces({4});
        updates.update_highlight_mask({.5f});
        auto &exhausted = const_cast<Drawable::BufferData &>(data(updates));
        exhausted.revision = std::numeric_limits<std::size_t>::max();
        const auto before = data(updates);
        const auto highlights = updates.highlight_mask();
        overflows([&] { updates.update_vertex_buffer({vec3(2, 2, 2)}); });
        overflows([&] { updates.update_color_buffer({vec3(0, 0, 0)}); });
        overflows([&] { updates.update_normal_buffer({vec3(0, 1, 0)}); });
        overflows([&] { updates.update_texcoord_buffer({vec2(1, 1)}); });
        overflows([&] { updates.update_element_buffer({2, 1, 0}); });
        overflows([&] { updates.disable_element_buffer(); });
        overflows([&] { updates.set_source_faces({8}); });
        overflows([&] { updates.update_highlight_mask({.25f}); });
        const auto &after = data(updates);
        require(after.vertices == before.vertices && after.colors == before.colors &&
                        after.normals == before.normals && after.texcoords == before.texcoords &&
                        after.indices == before.indices && after.source_faces == before.source_faces &&
                        after.revision == before.revision && updates.highlight_mask() == highlights &&
                        updates.num_vertices() == 3 && updates.num_indices() == 3,
                "a rejected public update partially changed drawable state");
    }

    void test_command_styles() {
        DrawableRenderer renderer;
        Camera camera;
        camera.set_screen_width_and_height(800, 600);

        auto points = std::make_shared<PointsDrawable>("points");
        points->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0)});
        points->update_normal_buffer({vec3(0, 0, 1), vec3(0, 0, 1)});
        points->set_point_size(7);
        auto command = renderer.command(points, 4, camera, {});
        require(command.point_style == PointStyle::plain && command.point_size == 7 &&
                        command.geometry->point_data()->source_ids()[1] == 1,
                "plain point command lost size or source-slot mapping");
        const auto plain_geometry = command.geometry;
        points->update_highlight_mask({.5f, .25f});
        command = renderer.command(points, 4, camera, {});
        require(command.geometry != plain_geometry &&
                        command.geometry->vertices()[0].highlight.w > 0,
                "highlight-only update did not invalidate the geometry cache");
        const auto highlighted_geometry = command.geometry;
        points->update_highlight_mask({});
        require(renderer.command(points, 4, camera, {}).geometry != highlighted_geometry,
                "clearing highlights did not invalidate the geometry cache");
        points->set_impostor_type(PointsDrawable::SPHERE);
        command = renderer.command(points, 4, camera, {});
        require(command.point_style == PointStyle::sphere && command.sphere_radius > 0,
                "sphere point command lost impostor state");
        points->set_impostor_type(PointsDrawable::SURFEL);
        command = renderer.command(points, 4, camera, {});
        require(command.point_style == PointStyle::surfel, "surfel point command lost its style");

        auto lines = std::make_shared<LinesDrawable>("lines");
        lines->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0)});
        lines->set_line_width(3);
        command = renderer.command(lines, 5, camera, {});
        require(command.line_style == LineStyle::plain && command.line_width == 3 &&
                        command.line_screen_space_width == setting::lines_drawable_screen_space_width,
                "plain line command lost width or sizing mode");
        lines->set_impostor_type(LinesDrawable::CYLINDER);
        command = renderer.command(lines, 5, camera, {});
        require(command.line_style == LineStyle::cylinder && command.line_radius > 0,
                "cylinder line command lost impostor state");

        auto triangles = std::make_shared<TrianglesDrawable>("triangles");
        triangles->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        triangles->update_normal_buffer({vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1)});
        triangles->set_smooth_shading(true);
        triangles->set_opacity(.25f);
        command = renderer.command(triangles, 6, camera, {});
        require(command.geometry->indices().empty() && !command.transparent &&
                        std::abs(command.color.w - triangles->color().w) < 1e-6f,
                "smooth triangle command lost shading state or enabled implicit transparency");
        triangles->set_smooth_shading(false);
        command = renderer.command(triangles, 6, camera, {});
        require(command.geometry->vertices().size() == 3 &&
                        command.geometry->vertices()[0].normal.z > 0,
                "flat triangle command did not derive face normals");
        rejects([&] { triangles->set_opacity(1.1f); }, "invalid opacity was accepted");
        rejects([&] { points->set_point_size(0); }, "invalid point size was accepted");

        auto screen = std::make_shared<LinesDrawable2D>("screen");
        screen->update_vertex_buffer({vec2(0, 0), vec2(40, 30)}, 800, 600);
        screen->set_line_width(2);
        command = renderer.command(screen, 7, camera, {});
        require(command.geometry->topology() == Topology::triangles && command.overlay &&
                        command.geometry->vertices().size() == 6 &&
                        std::abs(data(*screen).vertices[0].x + 1) < 1e-6f &&
                        std::abs(data(*screen).vertices[0].y - 1) < 1e-6f,
                "screen-space line command did not use portable NDC geometry");
        const auto cached_geometry = command.geometry;
        command = renderer.command(screen, 7, camera, {});
        require(command.geometry == cached_geometry, "unchanged screen-line geometry was not cached");
        const auto first_x = command.geometry->vertices()[0].position.x;
        screen->update_vertex_buffer({vec2(800, 600), vec2(0, 0)}, 800, 600);
        command = renderer.command(screen, 7, camera, {});
        require(command.geometry != cached_geometry &&
                        std::abs(data(*screen).vertices[0].x - 1) < 1e-6f &&
                        std::abs(data(*screen).vertices[0].y + 1) < 1e-6f &&
                        command.geometry->vertices()[0].position.x > first_x,
                "screen-space line cache did not follow a geometry update");

        auto provenance = std::make_shared<TrianglesDrawable>("triangle provenance");
        provenance->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0),
                                          vec3(0, 0, 1), vec3(1, 0, 1), vec3(0, 1, 1)});
        provenance->update_element_buffer({0, 1, 2, 3, 4, 5});
        provenance->set_source_faces({17, -1});
        provenance->set_smooth_shading(true);
        command = renderer.command(provenance, 8, camera, {});
        require(command.geometry->primitive_ids() == std::vector<uint32_t>({0, 1}),
                "triangle command lost per-primitive provenance slots");
        provenance->set_smooth_shading(false);
        command = renderer.command(provenance, 8, camera, {});
        require(command.geometry->primitive_ids() == std::vector<uint32_t>({0, 1}),
                "flat shading lost per-primitive provenance slots");
        provenance->update_highlight_mask({.5f, .25f});
        command = renderer.command(provenance, 8, camera, {});
        require(command.geometry->primitive_ids() == std::vector<uint32_t>({0, 1}),
                "highlight expansion lost per-primitive provenance slots");

        triangles->set_smooth_shading(true);
        triangles->update_element_buffer({0, 1, 2});
        const auto indexed_geometry = renderer.command(triangles, 6, camera, {}).geometry;
        require(indexed_geometry->indices().size() == 3, "triangle command lost portable indices");
        triangles->disable_element_buffer();
        const auto unindexed_geometry = renderer.command(triangles, 6, camera, {}).geometry;
        require(unindexed_geometry != indexed_geometry && unindexed_geometry->indices().empty(),
                "element-buffer disabling did not invalidate the geometry cache");
        triangles->update_vertex_buffer({vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0)});
        require(renderer.command(triangles, 6, camera, {}).geometry != unindexed_geometry,
                "a repeated vertex upload did not invalidate the geometry cache");
    }

} // namespace

int main() {
    try {
        test_geometry_updates();
        test_callback_realization_and_detachment();
        test_explicit_attributes_survive_realization();
        test_realized_index_mode_does_not_override_later_updates();
        test_realization_exception_recovery();
        test_standalone_virtual_realization();
        test_explicit_indices_clear_generated_provenance();
        test_first_use_provenance_and_highlights();
        test_const_realization_and_revision_exhaustion();
        test_command_styles();
        std::cout << "drawable contract tests passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "drawable contract test failed: " << error.what() << '\n';
        return 1;
    }
}
