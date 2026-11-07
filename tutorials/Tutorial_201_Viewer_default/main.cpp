
#include <easy3d/util/initializer.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/per_pixel_linked_lists_oit.h>



using namespace easy3d;
using Tech = PerPixelLinkedListsOIT;


class TransparencyViewer: public easy3d::Viewer {
public:
    explicit TransparencyViewer(const std::string& title = ""): Viewer(title) {
        transparency_ = new Tech(camera());
    }
    ~TransparencyViewer() override {
        delete transparency_;
    }

protected:
    bool draw() override {
        std::vector<TrianglesDrawable*> surfaces;
        for (auto d: drawables_) {
            surfaces.push_back(dynamic_cast<TrianglesDrawable*>(d.get()));
        }
        transparency_->draw(surfaces);
        return true;
    }

private:
    Tech* transparency_;
};


std::shared_ptr<TrianglesDrawable> make_cube(const vec3& offset) {
    std::vector<vec3> vertices = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
    };
    for (auto& v: vertices) v += offset;

    std::vector<unsigned int> indices = {
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        5, 4, 7, 5, 7, 6,
        4, 0, 3, 4, 3, 7,
        3, 2, 6, 3, 6, 7,
        4, 5, 1, 4, 1, 0
    };

    auto d = std::make_shared<TrianglesDrawable>("cube");
    d->update_vertex_buffer(vertices);
    d->update_element_buffer(indices);

    return d;
}

int main(int argc, char** argv) {
    initialize();

    TransparencyViewer viewer("Two Transparent Cubes");

    viewer.add_drawable(make_cube(vec3(0, 0, 0)));
    viewer.add_drawable(make_cube(vec3(0.5f, 0.5f, 0.5f)));

    return viewer.run();
}