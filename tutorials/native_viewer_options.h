// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TUTORIALS_NATIVE_VIEWER_OPTIONS_H
#define EASY3D_TUTORIALS_NATIVE_VIEWER_OPTIONS_H

#ifdef EASY3D_NO_OPENGL
#include <easy3d/viewer/viewer.h>
#include <easy3d/util/logging.h>
#include <cstdlib>
#include <limits>
#include <stdexcept>

namespace easy3d {
// Shared native execution options; tutorial scene construction stays identical
// to its OpenGL counterpart. Bounded execution supports desktop smoke tests.
struct NativeTutorialOptions {
    VulkanViewerOptions viewer;
    unsigned frames = 0;

    bool parse(int argc, char** argv) {
        try {
            for (int i = 1; i < argc; ++i) {
                const std::string argument(argv[i]);
                if (argument == "--validation") {
                    viewer.device.validation = viewer.device.synchronization_validation = true;
                } else if ((argument == "--frames" || argument == "--samples") && i+1 < argc) {
                    const std::string value(argv[++i]);
                    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
                        throw std::invalid_argument("expected a positive integer");
                    const auto count = std::stoul(value);
                    if (!count || count > std::numeric_limits<unsigned>::max())
                        throw std::invalid_argument("count out of range");
                    if (argument == "--frames") frames = static_cast<unsigned>(count);
                    else viewer.samples = static_cast<unsigned>(count);
                } else throw std::invalid_argument("usage: --frames N --samples N --validation");
            }
        } catch (const std::exception& e) {
            LOG(ERROR) << e.what();
            return false;
        }
        return true;
    }
    int run(VulkanViewer& host) const {
        const int result = host.run_frames(frames);
        if (viewer.device.validation && host.device().validation_errors()) {
            LOG(ERROR) << "Vulkan validation reported errors";
            return EXIT_FAILURE;
        }
        return result;
    }
};
}
#endif
#endif
