#ifndef EASY3D_VIEWER_H
#define EASY3D_VIEWER_H

#include <easy3d/viewer/viewer.h>
#include <easy3d/util/initializer.h>


// A naive example showing how to use Easy3D in Python"

class Easy3DViewer {
public:
    Easy3DViewer() = default;
    ~Easy3DViewer() = default;

    int run() {
        easy3d::initialize();
        easy3d::Viewer viewer("Easy3DViewer-Python");
        return viewer.run();
    }
};

#endif