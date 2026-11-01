// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TESTS_NATIVE_VIEWER_OPTIONS_H
#define EASY3D_TESTS_NATIVE_VIEWER_OPTIONS_H

#include <easy3d/viewer/viewer.h>
#include <easy3d/util/logging.h>
#include <cstdlib>
#include <limits>
#include <stdexcept>

namespace easy3d { namespace test {
// Options for test executables only. Interactive tutorials use Viewer::run(); the tests run them
// unchanged (a visible window, which the desktop tests already have under Xvfb) for a bounded
// number of frames.
struct NativeViewerOptions {
    unsigned frames = 0;

    bool parse(int argc, char** argv) {
        try {
            for (int i = 1; i < argc; ++i) {
                const std::string argument(argv[i]);
                if (argument == "--frames" && i+1 < argc) {
                    const std::string value(argv[++i]);
                    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
                        throw std::invalid_argument("expected a positive integer");
                    const auto count = std::stoul(value);
                    if (!count || count > std::numeric_limits<unsigned>::max())
                        throw std::invalid_argument("count out of range");
                    frames = static_cast<unsigned>(count);
                } else throw std::invalid_argument("usage: --frames N");
            }
        } catch (const std::exception& e) {
            LOG(ERROR) << e.what();
            return false;
        }
        return true;
    }
    int run(VulkanViewer& host) const {
        const int result = host.run_frames(frames);
        if (host.device().validation_errors()) {
            LOG(ERROR) << "Vulkan validation reported errors";
            return EXIT_FAILURE;
        }
        return result;
    }
};
inline NativeViewerOptions& native_options() {
    static NativeViewerOptions options;
    return options;
}
}}
#endif
