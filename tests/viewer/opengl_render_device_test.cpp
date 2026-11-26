// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/offscreen.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/drawable_renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/util/initializer.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace easy3d;
using namespace easy3d::graphics;

namespace {

    void require(bool value, const char *message) {
        if (!value)
            throw std::runtime_error(message);
    }

    template<class Function>
    void rejects(Function function) {
        bool threw = false;
        try {
            function();
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        require(threw, "invalid command was accepted");
    }

    DrawCommand rectangle(float x0, float y0, float x1, float y1, float z, const vec4 &color,
                          bool overlay = false, bool depth_test = false) {
        std::vector<Vertex> vertices(4);
        vertices[0].position = vec3(x0, y0, z);
        vertices[1].position = vec3(x1, y0, z);
        vertices[2].position = vec3(x1, y1, z);
        vertices[3].position = vec3(x0, y1, z);
        vertices[0].texcoord = vec2(0, 0);
        vertices[1].texcoord = vec2(1, 0);
        vertices[2].texcoord = vec2(1, 1);
        vertices[3].texcoord = vec2(0, 1);
        DrawCommand command;
        command.geometry = std::make_shared<Geometry>(std::move(vertices),
                                                     std::vector<uint32_t>{0, 1, 2, 0, 2, 3},
                                                     Topology::triangles, std::vector<uint32_t>{71, 72});
        command.color = color;
        command.overlay = overlay;
        command.overlay_depth_test = depth_test;
        return command;
    }

    size_t pixel(unsigned width, unsigned x, unsigned y) {
        return size_t(y) * width + x;
    }

    bool same_ids(const FrameImage &a, const FrameImage &b) {
        require(a.ids.size() == b.ids.size(), "picking buffer sizes differ");
        for (size_t i = 0; i < a.ids.size(); ++i)
            require(a.ids[i].object == b.ids[i].object && a.ids[i].primitive == b.ids[i].primitive,
                    "overlay changed scene picking IDs");
        return true;
    }

    void probe(const FrameImage &frame, unsigned x, unsigned y, int r, int g, int b, int a = 255) {
        const size_t offset = pixel(frame.width, x, y) * 4;
        const int expected[] = {r, g, b, a};
        for (unsigned c = 0; c < 4; ++c) {
            if (std::abs(int(frame.rgba[offset + c]) - expected[c]) > 1) {
                std::cerr << "color probe at " << x << "," << y << " channel " << c
                          << ": expected " << expected[c] << ", got " << int(frame.rgba[offset + c]) << std::endl;
                throw std::runtime_error("color probe mismatch");
            }
        }
    }

    void exercise(RenderDevice &device) {
        require(device.name().find("OpenGL") == 0, "OpenGL device name missing");
        require(!device.driver_version().empty(), "OpenGL driver version missing");
        require((device.supported_sample_counts() & 1) != 0, "single-sample count missing");

        const FrameImage blank = device.render(17, 13, {}, vec4(.1f, .2f, .3f, .4f));
        require(blank.width == 17 && blank.height == 13, "empty frame dimensions differ");
        require(blank.rgba.size() == 17 * 13 * 4 && blank.depth.size() == 17 * 13 && blank.ids.size() == 17 * 13,
                "empty frame readback sizes differ");
        probe(blank, 8, 6, 26, 51, 77, 102);
        require(blank.depth[pixel(17, 8, 6)] == 1, "empty scene depth was not cleared");
        require(blank.ids[pixel(17, 8, 6)].object == UINT32_MAX &&
                blank.ids[pixel(17, 8, 6)].primitive == UINT32_MAX, "empty scene IDs were not cleared");
        const FrameImage color_only = device.render(17, 13, {}, vec4(.1f, .2f, .3f, .4f), 1,
                                                    TransparencyMethod::sorted, 4, FrameReadback::color);
        require(color_only.depth.empty() && color_only.ids.empty(),
                "color-only readback copied scene data");
        require(color_only.rgba == blank.rgba, "color-only readback changed color pixels");

        auto scene = rectangle(-1, -1, 1, 1, 0, vec4(0, 0, 1, 1));
        scene.object_id = 13;
        const FrameImage frame = device.render(65, 65, {scene}, vec4(0));
        probe(frame, 10, 10, 0, 0, 255);
        probe(frame, 54, 54, 0, 0, 255);
        require(frame.ids[pixel(65, 32, 32)].object == 13 &&
                frame.ids[pixel(65, 32, 32)].primitive == 71, "triangle picking ID mismatch");
        require(std::abs(frame.depth[pixel(65, 32, 32)] - .5f) < 1e-6f, "OpenGL clip depth was converted twice");

        auto reordered = rectangle(-1, -1, 1, 1, 0, vec4(0, 0, 1, 1));
        reordered.object_id = 13;
        reordered.geometry = std::make_shared<Geometry>(scene.geometry->vertices(),
                                                       std::vector<uint32_t>{2, 1, 0, 3, 2, 0},
                                                       Topology::triangles, std::vector<uint32_t>{71, 72});
        const FrameImage reordered_frame = device.render(65, 65, {reordered}, vec4(0));
        unsigned first_triangle = 0, second_triangle = 0;
        for (const auto &id : reordered_frame.ids) {
            first_triangle += id.primitive == 71;
            second_triangle += id.primitive == 72;
        }
        require(first_triangle > 0 && second_triangle > 0, "indexed geometry lost exact primitive IDs");

        auto image = std::make_shared<Image>(2, 2, std::vector<unsigned char>{
                0, 255, 0, 255, 0, 255, 0, 255,
                255, 0, 0, 255, 255, 0, 0, 255});
        auto textured = rectangle(-1, -1, 1, 1, 0, vec4(1));
        textured.texture = image;
        const FrameImage texture_frame = device.render(65, 65, {textured}, vec4(0));
        probe(texture_frame, 32, 54, 255, 0, 0);
        probe(texture_frame, 32, 10, 0, 255, 0);

        auto hidden = rectangle(-1, -1, 1, 1, .5f, vec4(1, 0, 0, .5f), true, true);
        hidden.object_id = 99;
        auto visible = rectangle(-1, -1, 1, 1, -.5f, vec4(1, 0, 0, .5f), true, true);
        visible.object_id = 99;
        const FrameImage hidden_frame = device.render(65, 65, {hidden, scene}, vec4(0));
        probe(hidden_frame, 32, 32, 0, 0, 255);
        require(same_ids(hidden_frame, frame) && hidden_frame.depth == frame.depth,
                "depth annotation changed scene picking/depth");
        const FrameImage visible_frame = device.render(65, 65, {visible, scene}, vec4(0));
        probe(visible_frame, 32, 32, 128, 0, 128);
        require(same_ids(visible_frame, frame) && visible_frame.depth == frame.depth,
                "visible annotation changed scene picking/depth");
        auto flat = rectangle(-1, -1, 1, 1, 1, vec4(0, 1, 0, 1), true);
        probe(device.render(65, 65, {flat, visible, scene}, vec4(0)), 32, 32, 0, 255, 0);
        auto highlighted = rectangle(-1, -1, 1, 1, 1, vec4(0, 1, 0, 1), true);
        auto highlighted_vertices = highlighted.geometry->vertices();
        for (auto &vertex : highlighted_vertices)
            vertex.highlight = vec4(1, 0, 0, 1);
        highlighted.geometry = std::make_shared<Geometry>(std::move(highlighted_vertices),
                                                         highlighted.geometry->indices(), Topology::triangles);
        probe(device.render(65, 65, {highlighted, scene}, vec4(0)), 32, 32, 0, 255, 0);

        auto farther = rectangle(-1, -1, 1, 1, .5f, vec4(0, 0, 1, 1));
        farther.object_id = 21;
        farther.transparent = true;
        auto nearer = rectangle(-1, -1, 1, 1, -.5f, vec4(1, 0, 0, .5f));
        nearer.object_id = 22;
        nearer.transparent = true;
        const FrameImage sorted = device.render(65, 65, {nearer, farther}, vec4(0));
        probe(sorted, 32, 32, 128, 0, 128);
        require(sorted.ids[pixel(65, 32, 32)].object == 22, "nearest transparent surface was not pickable");

        Vertex point;
        point.position = vec3(0);
        DrawCommand points;
        points.geometry = std::make_shared<Geometry>(std::vector<Vertex>{point}, std::vector<uint32_t>{0},
                                                    Topology::points, std::vector<uint32_t>{87});
        points.object_id = 33;
        points.point_size = 12;
        points.color = vec4(1, 0, 0, 1);
        const FrameImage point_frame = device.render(64, 48, {points}, vec4(0));
        unsigned hits = 0;
        for (const auto &id : point_frame.ids) {
            if (id.object == 33) {
                ++hits;
                require(id.primitive == 87, "expanded point lost its primitive ID");
            }
        }
        require(hits == 144, "plain point is not an exact framebuffer-pixel square");

        if (device.supported_sample_counts() & 4) {
            auto msaa_scene = rectangle(-.8f, -.8f, .8f, .8f, 0, vec4(0, 0, 1, 1));
            msaa_scene.object_id = 13;
            const FrameImage msaa_reference = device.render(65, 65, {msaa_scene}, vec4(0));
            const FrameImage msaa = device.render(65, 65, {msaa_scene}, vec4(0), 4);
            unsigned partial = 0;
            for (size_t i = 0; i < msaa.ids.size(); ++i) {
                require(msaa.ids[i].object == msaa_reference.ids[i].object &&
                        msaa.ids[i].primitive == msaa_reference.ids[i].primitive, "MSAA changed picking IDs");
                require(std::abs(msaa.depth[i] - msaa_reference.depth[i]) < 1e-6f, "MSAA changed center depth");
                partial += msaa.rgba[4 * i + 2] > 0 && msaa.rgba[4 * i + 2] < 255;
            }
            require(partial > 0, "MSAA resolve has no partial edge coverage");
        }

        auto tile = rectangle(-1, -1, 1, 1, 0, vec4(1));
        tile.object_id = 44;
        tile.viewport = ivec4(2, 3, 4, 5);
        const FrameImage tiled = device.render(16, 16, {tile}, vec4(0));
        unsigned tile_hits = 0;
        for (const auto &id : tiled.ids)
            tile_hits += id.object == 44;
        require(tile_hits == 20, "top-left viewport/scissor conversion mismatch");
        require(tiled.ids[pixel(16, 2, 3)].object == 44 && tiled.ids[pixel(16, 14, 14)].object == UINT32_MAX,
                "top-left viewport origin mismatch");
        const auto region = device.sample_ids(16, 16, {tile}, 2, 3, 4, 5);
        require(region.size() == 20, "ID region size mismatch");
        for (unsigned y = 0; y < 5; ++y)
            for (unsigned x = 0; x < 4; ++x)
                require(region[y * 4 + x].object == tiled.ids[pixel(16, x + 2, y + 3)].object,
                        "ID region rows are not top-first");
        const ScenePixel center = device.sample_pixel(16, 16, {tile}, 3, 4);
        require(center.id.object == 44 && center.id.primitive == 72 && std::abs(center.depth - .5f) < 1e-6f,
                "single-pixel scene query mismatch");

        PointSelectionRequest selection;
        selection.width = selection.height = 100;
        selection.polygon = {{10, 10}, {90, 10}, {90, 90}, {10, 90}};
        selection.points = std::make_shared<PointData>(std::vector<vec3>{
                vec3(-.8f, .8f, 0), vec3(.8f, .8f, 0), vec3(.8f, -.8f, 0), vec3(-.8f, -.8f, 0), vec3(2, 0, 0)});
        selection.words = {0x15u};
        auto selected = device.select_points(selection);
        require(selected[0] == 0x1fu, "point lasso selection/padding mismatch");
        auto empty_words = selection;
        empty_words.words.clear();
        require(device.select_points(empty_words) == std::vector<uint32_t>{0x0fu},
                "empty selection bitset did not start from zero");
        const auto uploads = device.point_upload_count();
        require(device.select_points(selection) == selected && device.point_upload_count() == uploads,
                "point selection re-uploaded an unchanged snapshot");
        selection.words = selected;
        selection.deselect = true;
        require(device.select_points(selection)[0] == 0x10u, "point lasso deselection mismatch");

        auto cached_drawable = std::make_shared<PointsDrawable>("cached highlights");
        cached_drawable->update_vertex_buffer({vec3(0), vec3(1, 0, 0)});
        require(cached_drawable->num_vertices() == 2 && cached_drawable->num_indices() == 0,
                "highlight fixture has unexpected geometry counts");
        DrawableRenderer encoder;
        Camera camera;
        cached_drawable->update_highlight_mask({.25f, .25f});
        require(std::abs(encoder.command(cached_drawable, 0, camera, {}).geometry->vertices()[0].highlight.w - .25f) < 1e-6f,
                "initial highlight mask was not encoded");
        cached_drawable->update_highlight_mask({.75f, .75f});
        require(std::abs(encoder.command(cached_drawable, 0, camera, {}).geometry->vertices()[0].highlight.w - .75f) < 1e-6f,
                "highlight strength edit reused stale geometry");
        cached_drawable->update_highlight_mask({});
        require(encoder.command(cached_drawable, 0, camera, {}).geometry->vertices()[0].highlight.w == 0,
                "clearing the highlight mask reused stale geometry");

        GLint viewport[4], scissor_box[4];
        GLfloat clear_color[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor_box);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
        glViewport(7, 9, 11, 13);
        glScissor(23, 29, 31, 37);
        glEnable(GL_SCISSOR_TEST);
        glClearColor(.25f, .5f, .75f, 1);
        glEnable(GL_BLEND);
        glPolygonOffset(2, 3);
        glColorMaski(0, GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
        GLuint pack_buffer = 0;
        glGenBuffers(1, &pack_buffer);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pack_buffer);
        glBufferData(GL_PIXEL_PACK_BUFFER, 31 * 27 * 4, nullptr, GL_STREAM_READ);
        device.render(31, 27, {scene}, vec4(0));
        GLint restored_viewport[4], restored_scissor[4];
        GLfloat restored_clear[4];
        GLfloat restored_offset[2];
        GLboolean restored_mask[4];
        GLint restored_pack = 0;
        glGetIntegerv(GL_VIEWPORT, restored_viewport);
        glGetIntegerv(GL_SCISSOR_BOX, restored_scissor);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, restored_clear);
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &restored_offset[0]);
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &restored_offset[1]);
        glGetBooleani_v(GL_COLOR_WRITEMASK, 0, restored_mask);
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &restored_pack);
        for (unsigned i = 0; i < 4; ++i) {
            require(restored_viewport[i] == (i == 2 ? 11 : i == 3 ? 13 : i == 0 ? 7 : 9),
                    "device did not restore caller viewport");
            require(std::abs(restored_clear[i] - (i == 0 ? .25f : i == 1 ? .5f : i == 2 ? .75f : 1)) < 1e-6f,
                    "device did not restore caller clear color");
        }
        require(glIsEnabled(GL_BLEND), "device did not restore caller blend state");
        require(glIsEnabled(GL_SCISSOR_TEST) && restored_scissor[0] == 23 && restored_scissor[1] == 29 &&
                restored_scissor[2] == 31 && restored_scissor[3] == 37,
                "device did not restore caller scissor state");
        require(restored_offset[0] == 2 && restored_offset[1] == 3, "device did not restore polygon offset");
        require(restored_mask[0] == GL_FALSE && restored_mask[1] == GL_TRUE && restored_mask[2] == GL_FALSE &&
                restored_mask[3] == GL_TRUE, "device did not restore indexed color masks");
        require(restored_pack == GLint(pack_buffer), "device did not restore caller pixel-pack buffer");
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glDeleteBuffers(1, &pack_buffer);
        glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glPolygonOffset(0, 0);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissor_box[0], scissor_box[1], scissor_box[2], scissor_box[3]);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

        auto unsupported = points;
        unsupported.point_style = PointStyle::sphere;
        rejects([&] { device.render(32, 32, {unsupported}); });
        auto conflicting = visible;
        conflicting.overlay_independent_depth = true;
        rejects([&] { device.render(32, 32, {conflicting}); });
        rejects([&] { device.render(32, 32, {}, vec4(0), 1, TransparencyMethod::average); });
        require(device.validation_errors() == 3, "validation error count mismatch");
        require(device.oit_dropped_fragments() == 0, "unsupported OIT reported dropped fragments");
        bool present_rejected = false;
        try {
            device.present(32, 32, {});
        } catch (const std::logic_error &) {
            present_rejected = true;
        }
        require(present_rejected, "headless device accepted presentation");
        require(glGetError() == GL_NO_ERROR, "OpenGL device leaked a GL error");
    }

} // namespace

int main() {
    try {
        initialize(false, false, false);
        OffScreen host(64, 48);
        auto device = create_opengl_device();
        exercise(*device);
        std::cout << "PASS: OpenGL RenderDevice capture, overlays, transparency, MSAA, picking, selection and state isolation\n";
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
