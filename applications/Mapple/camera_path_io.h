#ifndef EASY3D_MAPPLE_CAMERA_PATH_IO_H
#define EASY3D_MAPPLE_CAMERA_PATH_IO_H

#include "generated_path.h"

#include <easy3d/renderer/keyframe_interpolator.h>

#include <string>
#include <vector>

namespace mapple {

    struct LoadedCameraPath {
        std::vector<GeneratedPath> generated_paths;
        CameraPath::Settings settings;
    };

    bool save_camera_path(const std::string &file_name,
                          const easy3d::KeyframeInterpolator &interpolator,
                          const std::vector<GeneratedPath> &generated_paths);

    bool read_camera_path(const std::string &file_name,
                          easy3d::KeyframeInterpolator *interpolator,
                          LoadedCameraPath &loaded);

} // namespace mapple

#endif
