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
with tempfile.TemporaryDirectory(prefix="easy3d-python-") as directory:
    root = Path(directory)
    mesh_file = root / "triangle.off"
    mesh_file.write_text("OFF\n3 1 0\n0 -1 -1\n0 1 -1\n0 0 1\n3 0 1 2\n")
    viewer = Window()
    viewer.set_usage("")
    model = add_triangle(viewer, mesh_file)
    assert viewer.samples() == 4 and viewer.run_frames(2) == 0
    assert viewer.before >= 2 and viewer.after == 2
    before, after = viewer.before, viewer.after
    point, found = viewer.point_under_pixel(viewer.width() // 2, viewer.height() // 2)
    assert found and abs(point.x) < 1e-4
    projected = viewer.project_point(point)
    restored = viewer.unproject_point(projected)
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
    override = DrawOverride()
    override.set_usage("")
    assert override.run_frames(2) == 0 and override.draws == 2
    del model, shared, offscreen, grid, viewer, override
    gc.collect()
print("Packaged Python Viewer, MultiViewer and OffScreen passed")
