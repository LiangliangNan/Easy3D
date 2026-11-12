"""Exercise the same packaged viewer APIs with either configured backend."""
import gc
import math
from pathlib import Path
import struct
import sys
import tempfile

sys.path.insert(0, sys.argv[1])
import easy3d


def bitmap(path):
    data = path.read_bytes()
    assert data[:2] == b"BM"
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    channels = struct.unpack_from("<H", data, 28)[0] // 8
    assert width > 0 and height != 0 and channels in (3, 4)
    stride = (width * channels + 3) & ~3

    def pixel(x, y):
        row = abs(height) - 1 - y if height > 0 else y
        pos = offset + row * stride + x * channels
        return tuple(data[pos:pos + 3][::-1])

    return width, abs(height), pixel


def add_triangle(host, path):
    model = host.add_model(str(path))
    assert model is not None and len(host.models()) == 1
    faces = model.renderer().get_triangles_drawable("faces")
    faces.set_lighting(False)
    faces.set_uniform_coloring(easy3d.vec4(1, 0, 0, 1))
    host.fit_screen()
    return model


def check_mesh_states(directory):
    # One front-facing and one back-facing triangle side by side in the plane
    # the default camera looks at. Both use their own vertices so normals and
    # primitive indices stay independent.
    path = directory / "two_faces.off"
    path.write_text("OFF\n6 2 0\n"
                    "0 -0.9 -0.6\n0 -0.9 0.6\n0 -0.3 0\n"
                    "0 0.3 -0.6\n0 0.3 0.6\n0 0.9 0\n"
                    "3 0 1 2\n3 3 5 4\n")
    viewer = easy3d.Viewer("Python mesh state", width=181, height=151, samples=1, visible=False)
    viewer.set_usage("")
    viewer.set_background_color(easy3d.vec4(0, 0, 0, 1))
    model = viewer.add_model(str(path))
    faces = model.renderer().get_triangles_drawable("faces")
    assert faces is not None
    faces.set_uniform_coloring(easy3d.vec4(0, 0, 1, 1))
    faces.set_material(easy3d.State.Material(easy3d.vec4(0, 0, 0, 0), easy3d.vec4(0, 0, 0, 0), 1))
    faces.set_smooth_shading(True)
    assert viewer.run_frames(1) == 0

    def probes(name, visible=True):
        image = directory / name
        assert viewer.snapshot(str(image), 1, 1, 0, True)
        w, h, pixel = bitmap(image)
        scale_x, scale_y = w / viewer.width(), h / viewer.height()

        def probe(model_y, model_z):
            screen = viewer.world_to_window(easy3d.vec3(0, model_y, model_z))
            x, y = int(round(screen.x)), int(round(screen.y))
            point, found = viewer.point_under_pixel(x, y)
            assert found == visible and (not found or abs(point.x) < 1e-4)
            return pixel(int(round(x * scale_x)), int(round(y * scale_y)))

        # Symmetric interior points of the two triangles. The upper half of the
        # viewport keeps them clear of the lower-left orientation-axes HUD, which
        # Vulkan file snapshots include by design.
        return probe(-0.8, 0.3), probe(0.4, 0.3)

    def blue(p):
        return p[2] > 60 and p[2] > 2 * p[0] and p[2] > 2 * p[1]

    def green(p):
        return p[1] > 60 and p[1] > 2 * p[0] and p[1] > 2 * p[2]

    # Unlit rendering is the exact uniform color: back-face color is ignored.
    faces.set_lighting(False)
    faces.set_distinct_back_color(True)
    faces.set_back_color(easy3d.vec4(0, 1, 0, 1))
    unlit = probes("state_unlit.bmp")
    assert unlit == ((0, 0, 255), (0, 0, 255))

    # Selection and primitive-range highlighting tint the final color with or
    # without lighting.
    faces.set_selected(True)
    unlit_selected = probes("state_unlit_selected.bmp")
    assert unlit_selected[0] != unlit[0] and unlit_selected[1] != unlit[1]
    faces.set_selected(False)
    faces.set_highlight(True)
    faces.set_highlight_range((0, 1))
    unlit_range = probes("state_unlit_range.bmp")
    assert unlit_range[0] != unlit[0] and unlit_range[1] != unlit[1]
    faces.set_highlight(False)
    faces.set_highlight_range((-1, -1))

    # Lighting: the distinct back color must reach exactly the back-facing side.
    faces.set_lighting(True)
    faces.set_lighting_two_sides(True)
    back = probes("state_back.bmp")
    assert (blue(back[0]) and green(back[1])) or (green(back[0]) and blue(back[1]))
    faces.set_distinct_back_color(False)
    assert all(blue(p) for p in probes("state_front.bmp"))
    faces.set_distinct_back_color(True)

    # Range highlighting follows rasterized primitive order: each range tints one
    # triangle and leaves the other untouched.
    faces.set_highlight(True)
    faces.set_highlight_range((0, 0))
    first = probes("state_range0.bmp")
    assert (first[0] != back[0]) != (first[1] != back[1])
    faces.set_highlight_range((1, 1))
    second = probes("state_range1.bmp")
    assert (second[0] != back[0]) != (second[1] != back[1])
    assert (first[0] == back[0]) != (second[0] == back[0])
    faces.set_highlight_range((-1, -1))
    faces.set_highlight(False)
    assert probes("state_clear.bmp") == back

    # Whole-drawable selection and visibility keep depth/IDs unchanged.
    faces.set_selected(True)
    selected = probes("state_selected.bmp")
    assert selected[0] != back[0] and selected[1] != back[1]
    faces.set_selected(False)
    assert probes("state_unselected.bmp") == back
    faces.set_visible(False)
    assert probes("state_hidden.bmp", False) == ((0, 0, 0), (0, 0, 0))
    faces.set_visible(True)
    assert probes("state_visible.bmp") == back
    viewer.clear_scene()
    del viewer


def check_raster_states(directory):
    # OffScreen images are scene-only on both backends (no logo, axes, or
    # hints), so the probes below are not affected by backend HUD differences.
    viewer = easy3d.OffScreen(129, 97)
    viewer.set_background_color(easy3d.vec4(0, 0, 0, 1))
    camera = viewer.camera()
    camera.set_type(easy3d.Camera.ORTHOGRAPHIC)
    camera.set_scene_bounding_box(easy3d.vec3(-1), easy3d.vec3(1))
    camera.set_view_direction(easy3d.vec3(0, 0, -1))
    camera.set_up_vector(easy3d.vec3(0, 1, 0))
    camera.show_entire_scene()

    def probes(name, points):
        image = directory / name
        assert viewer.render(str(image), 1, 1, 0, True)
        w, h, pixel = bitmap(image)
        assert (w, h) == (viewer.width(), viewer.height())

        def sample(world):
            # projected_coordinates_of() uses the same top-down rows as bitmap().
            screen = camera.projected_coordinates_of(world)
            return pixel(int(round(screen.x)), int(round(screen.y)))

        return [sample(point) for point in points]

    def red(p):
        return p[0] > 120 and p[0] > 2 * p[1] and p[0] > 2 * p[2]

    def green(p):
        return p[1] > 120 and p[1] > 2 * p[0] and p[1] > 2 * p[2]

    def blue(p):
        return p[2] > 120 and p[2] > 2 * p[0] and p[2] > 2 * p[1]

    def yellow(p):
        return p[0] > 150 and p[1] > 150 and p[2] < 80

    # Per-vertex colors interpolate across the primitive identically on both
    # backends. Probes sit 80% toward each vertex, where that vertex dominates.
    triangle = easy3d.TrianglesDrawable("vertex colors")
    triangle.update_vertex_buffer([easy3d.vec3(-0.8, -0.8, 0), easy3d.vec3(0.8, -0.8, 0),
                                   easy3d.vec3(0, 0.8, 0)])
    triangle.update_color_buffer([easy3d.vec3(1, 0, 0), easy3d.vec3(0, 1, 0), easy3d.vec3(0, 0, 1)])
    triangle.set_coloring_method(easy3d.State.COLOR_PROPERTY)
    triangle.set_lighting(False)
    assert viewer.add_drawable(triangle)
    colors = probes("raster_vertex_colors.bmp",
                    [easy3d.vec3(-0.64, -0.64, 0), easy3d.vec3(0.64, -0.64, 0),
                     easy3d.vec3(0, 0.64, 0)])
    assert red(colors[0]) and green(colors[1]) and blue(colors[2])
    assert viewer.delete_drawable(triangle)

    # A 4x1 texture with a distinct texel per quarter. Texel centers keep the
    # LINEAR filtering of the file-loaded texture to a single texel.
    texture = directory / "raster_4x1.bmp"
    assert easy3d.ImageIO.save(str(texture), [255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0],
                               4, 1, 3, True)
    quad = easy3d.TrianglesDrawable("textured")
    quad.update_vertex_buffer([easy3d.vec3(-0.8, -0.8, 0), easy3d.vec3(0.8, -0.8, 0),
                               easy3d.vec3(0.8, 0.8, 0), easy3d.vec3(-0.8, 0.8, 0)])
    quad.update_texcoord_buffer([easy3d.vec2(0, 0), easy3d.vec2(1, 0),
                                 easy3d.vec2(1, 1), easy3d.vec2(0, 1)])
    quad.update_element_buffer([[0, 1, 2], [0, 2, 3]])
    quad.set_lighting(False)
    quad.set_texture(str(texture))
    quad.set_coloring_method(easy3d.State.TEXTURED)
    assert viewer.add_drawable(quad)

    clamp = probes("raster_texture_clamp.bmp",
                   [easy3d.vec3(x, 0, 0) for x in (-0.6, -0.2, 0.2, 0.6)])
    assert red(clamp[0]) and green(clamp[1]) and blue(clamp[2]) and yellow(clamp[3])

    # The repeat factor scales the texcoords, and the REPEAT wrap makes the
    # sampling periodic: one period is 1/3 of the quad.
    quad.set_texture(str(texture), True)
    quad.set_texture_repeat(3)
    repeated = probes("raster_texture_repeat.bmp",
                      [easy3d.vec3(x, 0, 0) for x in (-0.6, -0.6 + 1.6 / 3, -0.6 + 3.2 / 3)])
    assert all(p[1] > 120 and p[1] > p[0] and p[1] > p[2] for p in repeated)
    assert not red(repeated[0])
    assert max(max(abs(a - b) for a, b in zip(repeated[0], p)) for p in repeated) <= 16
    assert viewer.delete_drawable(quad)
    viewer.clear_scene()


def check_point_line_states(directory):
    # Same scene-only DPI-1 harness as check_raster_states(). The default
    # highlight color is red, so base colors avoid red wherever a tint must
    # be observable.
    viewer = easy3d.OffScreen(129, 97)
    viewer.set_background_color(easy3d.vec4(0, 0, 0, 1))
    camera = viewer.camera()

    def frame():
        camera.set_type(easy3d.Camera.ORTHOGRAPHIC)
        camera.set_scene_bounding_box(easy3d.vec3(-1), easy3d.vec3(1))
        camera.set_view_direction(easy3d.vec3(0, 0, -1))
        camera.set_up_vector(easy3d.vec3(0, 1, 0))
        camera.show_entire_scene()

    frame()

    def image(name):
        path = directory / name
        assert viewer.render(str(path), 1, 1, 0, True)
        w, h, pixel = bitmap(path)
        assert (w, h) == (viewer.width(), viewer.height())
        return pixel

    def at(pixel, world, dx=0, dy=0):
        screen = camera.projected_coordinates_of(world)
        return pixel(int(round(screen.x)) + dx, int(round(screen.y)) + dy)

    def thin(pixel, world):
        # Size-2 points and width-1 lines may rasterize into either adjacent
        # pixel depending on the sub-pixel center; take the strongest neighbor.
        screen = camera.projected_coordinates_of(world)
        x, y = int(round(screen.x)), int(round(screen.y))
        return max((pixel(x + dx, y + dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1)), key=sum)

    def blank(pixel, world, dy):
        screen = camera.projected_coordinates_of(world)
        x, y = int(round(screen.x)), int(round(screen.y))
        return all(pixel(x + dx, y + dy + j) == (0, 0, 0)
                   for dx in (-1, 0, 1) for j in (-1, 0, 1))

    def green(p):
        return p[1] > 120 and p[1] > 2 * p[0] and p[1] > 2 * p[2]

    def blue(p):
        return p[2] > 120 and p[2] > 2 * p[0] and p[2] > 2 * p[1]

    left, right = easy3d.vec3(-0.5, 0.3, 0), easy3d.vec3(0.5, 0.3, 0)

    # Plain points: uniform color, point size coverage, per-point colors,
    # selection and primitive-range highlighting without lighting, visibility.
    points = easy3d.PointsDrawable("plain points")
    points.update_vertex_buffer([left, right])
    points.set_lighting(False)
    points.set_uniform_coloring(easy3d.vec4(0, 0, 1, 1))
    assert viewer.add_drawable(points)
    pixel = image("points_plain_small.bmp")
    assert thin(pixel, left) == (0, 0, 255) and thin(pixel, right) == (0, 0, 255)
    assert blank(pixel, left, 8)
    points.set_point_size(21)
    pixel = image("points_plain_large.bmp")
    assert at(pixel, left, 0, 8) == (0, 0, 255) and at(pixel, right, 0, -8) == (0, 0, 255)
    assert at(pixel, left, 0, 14) == (0, 0, 0)
    points.update_color_buffer([easy3d.vec3(0, 1, 0), easy3d.vec3(0, 0, 1)])
    points.set_coloring_method(easy3d.State.COLOR_PROPERTY)
    pixel = image("points_plain_colors.bmp")
    assert thin(pixel, left) == (0, 255, 0) and thin(pixel, right) == (0, 0, 255)
    points.set_selected(True)
    selected = image("points_plain_selected.bmp")
    assert thin(selected, left) != (0, 255, 0) and thin(selected, right) != (0, 0, 255)
    points.set_selected(False)
    # An unlit primitive range tints exactly one point.
    points.set_highlight(True)
    points.set_highlight_range((0, 0))
    first = image("points_plain_range0.bmp")
    assert thin(first, left) != (0, 255, 0) and thin(first, right) == (0, 0, 255)
    points.set_highlight_range((1, 1))
    second = image("points_plain_range1.bmp")
    assert thin(second, left) == (0, 255, 0) and thin(second, right) != (0, 0, 255)
    points.set_highlight_range((-1, -1))
    points.set_highlight(False)
    pixel = image("points_plain_clear.bmp")
    assert thin(pixel, left) == (0, 255, 0) and thin(pixel, right) == (0, 0, 255)
    points.set_visible(False)
    pixel = image("points_plain_hidden.bmp")
    assert thin(pixel, left) == (0, 0, 0) and thin(pixel, right) == (0, 0, 0)
    points.set_visible(True)
    assert viewer.delete_drawable(points)

    # Sphere impostors: the orthographic radius is point_size/2 pixels on both
    # backends (sphere_radius = point_size * world_pixel_ratio / 2).
    spheres = easy3d.PointsDrawable("sphere points")
    spheres.update_vertex_buffer([left, right])
    spheres.set_impostor_type(easy3d.PointsDrawable.SPHERE)
    spheres.set_point_size(25)
    spheres.set_lighting(False)
    spheres.set_uniform_coloring(easy3d.vec4(0, 1, 0, 1))
    assert viewer.add_drawable(spheres)
    pixel = image("points_sphere_unlit.bmp")
    assert at(pixel, left) == (0, 255, 0) and at(pixel, right) == (0, 255, 0)
    assert at(pixel, left, 0, 10) == (0, 255, 0)
    assert at(pixel, left, 0, 16) == (0, 0, 0)
    spheres.set_highlight(True)
    spheres.set_highlight_range((0, 0))
    first = image("points_sphere_range0.bmp")
    assert at(first, left) != (0, 255, 0) and at(first, right) == (0, 255, 0)
    spheres.set_highlight_range((-1, -1))
    spheres.set_highlight(False)
    spheres.set_lighting(True)
    spheres.set_material(easy3d.State.Material(easy3d.vec4(0, 0, 0, 0), easy3d.vec4(0, 0, 0, 0), 1))
    pixel = image("points_sphere_lit.bmp")
    assert green(at(pixel, left)) and green(at(pixel, right))
    assert at(pixel, left, 0, 16) == (0, 0, 0)
    assert viewer.delete_drawable(spheres)

    # Plain lines of width 1: uniform color, per-vertex interpolation along the
    # segment, and primitive-range highlighting (plain lines are never lit).
    a, b = easy3d.vec3(-0.7, -0.3, 0), easy3d.vec3(-0.1, -0.3, 0)
    c, d = easy3d.vec3(0.1, -0.3, 0), easy3d.vec3(0.7, -0.3, 0)
    mid_ab, mid_cd = easy3d.vec3(-0.4, -0.3, 0), easy3d.vec3(0.4, -0.3, 0)
    lines = easy3d.LinesDrawable("plain lines")
    lines.update_vertex_buffer([a, b, c, d])
    lines.set_lighting(False)
    lines.set_uniform_coloring(easy3d.vec4(0, 1, 0, 1))
    assert viewer.add_drawable(lines)
    pixel = image("lines_plain_thin.bmp")
    assert thin(pixel, mid_ab) == (0, 255, 0) and thin(pixel, mid_cd) == (0, 255, 0)
    assert blank(pixel, mid_ab, 5)
    lines.update_color_buffer([easy3d.vec3(1, 0, 0), easy3d.vec3(0, 0, 1),
                               easy3d.vec3(0, 0, 1), easy3d.vec3(1, 0, 0)])
    lines.set_coloring_method(easy3d.State.COLOR_PROPERTY)
    pixel = image("lines_plain_colors.bmp")
    p = thin(pixel, mid_ab)
    assert p[0] > 60 and p[2] > 60 and p[1] < 60
    lines.set_highlight(True)
    lines.set_highlight_range((0, 0))
    first = image("lines_plain_range0.bmp")
    assert thin(first, mid_ab) != p and thin(first, mid_cd) == thin(pixel, mid_cd)
    lines.set_highlight_range((-1, -1))
    lines.set_highlight(False)
    lines.set_selected(True)
    selected = image("lines_plain_selected.bmp")
    assert thin(selected, mid_ab) != p and thin(selected, mid_cd) != thin(pixel, mid_cd)
    lines.set_selected(False)
    # Plain lines are never lit: lines_plain_color.frag has no lit branch, so
    # enabling lighting must not change the native width-1 rasterization.
    lines.set_lighting(True)
    lit = image("lines_plain_lit.bmp")
    assert thin(lit, mid_ab) == p and thin(lit, mid_cd) == thin(pixel, mid_cd)
    lines.set_lighting(False)

    # Wide plain lines use the width-control shader: the width extends the
    # coverage, primitive ranges tint individual segments, and lighting stays
    # irrelevant (plain lines are never lit).
    lines.set_line_width(9)
    pixel = image("lines_plain_wide.bmp")
    assert at(pixel, mid_ab, 0, 3) == at(pixel, mid_ab)
    assert at(pixel, mid_ab, 0, 8) == (0, 0, 0)
    lines.set_highlight(True)
    lines.set_highlight_range((0, 0))
    wide_range = image("lines_plain_wide_range.bmp")
    assert at(wide_range, mid_ab) != at(pixel, mid_ab)
    assert at(wide_range, mid_cd) == at(pixel, mid_cd)
    lines.set_highlight_range((-1, -1))
    lines.set_highlight(False)
    lines.set_lighting(True)
    lit = image("lines_plain_wide_lit.bmp")
    assert at(lit, mid_ab) == at(pixel, mid_ab) and at(lit, mid_cd) == at(pixel, mid_cd)
    lines.set_lighting(False)
    assert viewer.delete_drawable(lines)

    # Cylinder impostors: selection and primitive ranges tint the final color
    # with or without lighting.
    cylinders = easy3d.LinesDrawable("cylinder lines")
    cylinders.update_vertex_buffer([a, b, c, d])
    cylinders.set_impostor_type(easy3d.LinesDrawable.CYLINDER)
    cylinders.set_line_width(15)
    cylinders.set_uniform_coloring(easy3d.vec4(0, 0, 1, 1))
    cylinders.set_material(easy3d.State.Material(easy3d.vec4(0, 0, 0, 0), easy3d.vec4(0, 0, 0, 0), 1))
    assert viewer.add_drawable(cylinders)
    pixel = image("lines_cylinder_lit.bmp")
    assert blue(at(pixel, mid_ab)) and blue(at(pixel, mid_cd))
    assert at(pixel, mid_ab, 0, 10) == (0, 0, 0)
    cylinders.set_highlight(True)
    cylinders.set_highlight_range((0, 0))
    lit_range = image("lines_cylinder_lit_range.bmp")
    assert at(lit_range, mid_ab) != at(pixel, mid_ab)
    assert at(lit_range, mid_cd) == at(pixel, mid_cd)
    cylinders.set_highlight_range((-1, -1))
    cylinders.set_highlight(False)
    cylinders.set_selected(True)
    lit_selected = image("lines_cylinder_lit_selected.bmp")
    assert at(lit_selected, mid_ab) != at(pixel, mid_ab)
    cylinders.set_selected(False)
    cylinders.set_lighting(False)
    pixel = image("lines_cylinder_unlit.bmp")
    assert at(pixel, mid_ab) == (0, 0, 255) and at(pixel, mid_cd) == (0, 0, 255)
    cylinders.set_selected(True)
    unlit_selected = image("lines_cylinder_unlit_selected.bmp")
    assert at(unlit_selected, mid_ab) != (0, 0, 255) and at(unlit_selected, mid_cd) != (0, 0, 255)
    cylinders.set_selected(False)
    cylinders.set_highlight(True)
    cylinders.set_highlight_range((0, 0))
    unlit_range = image("lines_cylinder_unlit_range.bmp")
    assert at(unlit_range, mid_ab) != (0, 0, 255) and at(unlit_range, mid_cd) == (0, 0, 255)
    cylinders.set_highlight_range((-1, -1))
    cylinders.set_highlight(False)
    assert viewer.delete_drawable(cylinders)

    # Cone impostors follow the same highlight contract as cylinders.
    cones = easy3d.LinesDrawable("cone lines")
    cones.update_vertex_buffer([a, b, c, d])
    cones.set_impostor_type(easy3d.LinesDrawable.CONE)
    cones.set_line_width(15)
    cones.set_lighting(False)
    cones.set_uniform_coloring(easy3d.vec4(0, 0, 1, 1))
    cones.set_material(easy3d.State.Material(easy3d.vec4(0, 0, 0, 0), easy3d.vec4(0, 0, 0, 0), 1))
    assert viewer.add_drawable(cones)
    pixel = image("lines_cone_unlit.bmp")
    assert at(pixel, mid_ab) == (0, 0, 255) and at(pixel, mid_cd) == (0, 0, 255)
    cones.set_highlight(True)
    cones.set_highlight_range((1, 1))
    cone_range = image("lines_cone_range.bmp")
    assert at(cone_range, mid_ab) == (0, 0, 255) and at(cone_range, mid_cd) != (0, 0, 255)
    cones.set_highlight_range((-1, -1))
    cones.set_highlight(False)
    cones.set_selected(True)
    cone_selected = image("lines_cone_selected.bmp")
    assert at(cone_selected, mid_ab) != (0, 0, 255) and at(cone_selected, mid_cd) != (0, 0, 255)
    cones.set_selected(False)
    assert viewer.delete_drawable(cones)

    # The tutorial API path: a PointCloud built in Python exposes its default
    # "vertices" drawable, and a loaded mesh exposes "edges" (Tutorial 204).
    cloud = easy3d.PointCloud([left, right])
    assert viewer.add_model(cloud) is not None
    frame()
    vertices = cloud.renderer().get_points_drawable("vertices")
    assert vertices is not None
    vertices.set_impostor_type(easy3d.PointsDrawable.SPHERE)
    vertices.set_point_size(25)
    vertices.set_lighting(False)
    vertices.set_uniform_coloring(easy3d.vec4(0, 1, 0, 1))
    pixel = image("cloud_vertices.bmp")
    assert at(pixel, left) == (0, 255, 0) and at(pixel, right) == (0, 255, 0)
    assert viewer.delete_model(cloud)

    mesh_file = directory / "edge_mesh.off"
    mesh_file.write_text("OFF\n3 1 0\n0 -0.9 -0.6\n0 -0.9 0.6\n0 -0.3 0\n3 0 1 2\n")
    mesh = viewer.add_model(str(mesh_file))
    assert mesh is not None
    frame()
    faces = mesh.renderer().get_triangles_drawable("faces")
    edges = mesh.renderer().get_lines_drawable("edges")
    assert faces is not None and edges is not None
    faces.set_visible(False)
    edges.set_visible(True)
    edges.set_impostor_type(easy3d.LinesDrawable.CYLINDER)
    edges.set_line_width(15)
    edges.set_uniform_coloring(easy3d.vec4(0, 0, 1, 1))
    edges.set_material(easy3d.State.Material(easy3d.vec4(0, 0, 0, 0), easy3d.vec4(0, 0, 0, 0), 1))
    pixel = image("mesh_edges.bmp")
    assert blue(at(pixel, easy3d.vec3(0, -0.6, 0.3)))
    assert at(pixel, easy3d.vec3(0.5, 0.5, 0)) == (0, 0, 0)
    viewer.clear_scene()


class Window(easy3d.Viewer):
    def __init__(self):
        super().__init__("Python Viewer", width=181, height=151, samples=4, visible=False)
        self.before = self.after = 0

    def pre_draw(self):
        self.before += 1

    def post_draw(self):
        self.after += 1


class Grid(easy3d.MultiViewer):
    def __init__(self):
        super().__init__(1, 2, "Python MultiViewer", width=301, height=151, visible=False)
        self.snapshots = 0

    def snapshot(self, file, scale, samples, background, expand):
        self.snapshots += 1
        return super().snapshot(file, scale, samples, background, expand)


class DrawOverride(easy3d.Viewer):
    def __init__(self):
        super().__init__("Python draw override", width=81, height=61, visible=False)
        self.draws = 0

    def draw(self):
        self.draws += 1
        return True


easy3d.initialize(False)
# Signed constructor arguments reach C++ validation instead of unsigned conversion.
for constructor, arguments in (
    (easy3d.Viewer, dict(width=-1, visible=False)),
    (easy3d.Viewer, dict(height=0, visible=False)),
    (easy3d.Viewer, dict(samples=-1, visible=False)),
    (easy3d.Viewer, dict(samples=-(2**31), visible=False)),
    (easy3d.OffScreen, dict(width=-1)),
    (easy3d.OffScreen, dict(height=0)),
    (easy3d.MultiViewer, dict(rows=1, cols=2, width=-1, visible=False)),
    (easy3d.MultiViewer, dict(rows=1, cols=2, samples=-1, visible=False)),
    (easy3d.MultiViewer, dict(rows=-1, cols=2, width=31, height=23, visible=False)),
):
    try:
        constructor(**arguments)
    except ValueError:
        pass
    else:
        raise AssertionError("invalid signed constructor argument was accepted")

zero_samples = easy3d.Viewer("No MSAA", width=31, height=23, samples=0, visible=False)
assert zero_samples.samples() == 1
del zero_samples

with tempfile.TemporaryDirectory(prefix="easy3d-python-") as directory:
    root = Path(directory)
    mesh_file = root / "triangle.off"
    mesh_file.write_text("OFF\n3 1 0\n0 -1 -1\n0 1 -1\n0 0 1\n3 0 1 2\n")
    viewer = Window()
    viewer.set_usage("")
    model = add_triangle(viewer, mesh_file)
    # Software rasterizers (for example llvmpipe under Xvfb) expose no multisample visual,
    # and the viewer reports the single sample it actually got.
    assert viewer.samples() in (1, 4) and viewer.run_frames(2) == 0
    assert viewer.before >= 2 and viewer.after == 2
    before, after = viewer.before, viewer.after
    point, found = viewer.point_under_pixel(viewer.width() // 2, viewer.height() // 2)
    assert found and abs(point.x) < 1e-4
    projected = viewer.world_to_window(point)
    restored = viewer.window_to_world(projected)
    assert sum((getattr(restored, c) - getattr(point, c)) ** 2 for c in "xyz") < 1e-8
    image = root / "viewer.bmp"
    assert viewer.snapshot(str(image), 1, 4, 1, True)
    w, h, pixel = bitmap(image)
    assert (w, h) == (viewer.framebuffer_width(), viewer.framebuffer_height())
    assert pixel(w // 2, h // 2) == (255, 0, 0)
    assert (viewer.before, viewer.after) == (before, after)
    assert viewer.models()[0] is model
    viewer.clear_scene()
    assert not viewer.models() and model.n_vertices() == 3

    grid = Grid()
    grid.set_usage("")
    shared = add_triangle(grid, mesh_file)
    grid.assign(0, 0, shared)
    grid.assign(0, 1, shared)
    grid.set_division_visible(False)
    assert grid.run_frames(2) == 0
    image = root / "grid.bmp"
    assert easy3d.Viewer.snapshot(grid, str(image), 1.25, 4, 1, True)
    assert grid.snapshots == 1
    w, h, pixel = bitmap(image)
    assert (w, h) == (math.ceil(grid.framebuffer_width() * 1.25), math.ceil(grid.framebuffer_height() * 1.25))
    for x in (w // 4, 3 * w // 4):
        assert pixel(x, h // 2) == (255, 0, 0)
    for x in (grid.width() // 4, 3 * grid.width() // 4):
        point, found = grid.point_under_pixel(x, grid.height() // 2)
        assert found and abs(point.x) < 1e-4
    grid.set_layout(2, 2)
    assert grid.rows() == 2 and grid.columns() == 2
    grid.clear_scene()
    assert shared.n_vertices() == 3

    offscreen = easy3d.OffScreen(101, 77)
    model = add_triangle(offscreen, mesh_file)
    image = root / "offscreen.bmp"
    assert offscreen.render(str(image), 1, 4, 1, True)
    w, h, pixel = bitmap(image)
    assert (w, h) == (101, 77) and pixel(w // 2, h // 2) == (255, 0, 0)
    assert offscreen.delete_model(model) and not offscreen.models()
    assert model.n_vertices() == 3

    check_mesh_states(root)
    check_raster_states(root)
    check_point_line_states(root)

    override = DrawOverride()
    override.set_usage("")
    assert override.run_frames(2) == 0 and override.draws == 2
    del model, shared, offscreen, grid, viewer, override
    gc.collect()
print("Packaged Python Viewer, MultiViewer and OffScreen passed")
