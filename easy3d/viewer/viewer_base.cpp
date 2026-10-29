// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer_base.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/backends/viewer.h>
#if defined(EASY3D_HAS_VULKAN_VIEWER) || !defined(EASY3D_NO_OPENGL)
#include <easy3d/viewer/backends/glfw_runtime.h>
#endif
#include <cmath>
#include <limits>
namespace easy3d {
ViewerBase::~ViewerBase() = default;
namespace {
std::unique_ptr<ViewerBase> create_backend(Backend backend, const std::string& title, const ViewerOptions& o) {
    switch (backend) {
    case Backend::Vulkan:
#ifdef EASY3D_HAS_VULKAN_VIEWER
        return detail::create_vulkan_viewer(title, o);
#else
        throw BackendUnavailable("Vulkan window support was not built");
#endif
    case Backend::OpenGL:
#ifndef EASY3D_NO_OPENGL
        return detail::create_opengl_viewer(title, o);
#else
        throw BackendUnavailable("OpenGL window support was not built");
#endif
    default: throw std::invalid_argument("invalid viewer backend");
    }
}
}
std::unique_ptr<ViewerBase> ViewerBase::create(const std::string& title, const ViewerOptions& options) {
    if (options.width <= 0 || options.height <= 0)
        throw std::invalid_argument("viewer dimensions must be positive");
    if (!options.samples || options.samples > 64 || (options.samples & (options.samples - 1)))
        throw std::invalid_argument("viewer samples must be a power of two between 1 and 64");
    std::unique_ptr<ViewerBase> host;
    std::string fallback;
    if (options.backend == Backend::Automatic) {
        try { host = create_backend(Backend::Vulkan, title, options); }
        catch (const BackendUnavailable& e) {
            fallback = e.what();
            try { host = create_backend(Backend::OpenGL, title, options); }
            catch (const BackendUnavailable& gl) {
                throw BackendUnavailable("No interactive backend available. Vulkan: " + fallback + "; OpenGL: " + gl.what());
            }
        }
    } else host = create_backend(options.backend, title, options);
    host->fallback_reason_ = std::move(fallback);
    // Match the original Viewer defaults before applications configure a scene.
    auto* camera = host->camera();
    camera->setType(Camera::PERSPECTIVE);
    camera->setUpVector(vec3(0, 0, 1));
    camera->setViewDirection(vec3(-1, 0, 0));
    camera->showEntireScene();
    if (options.visible) host->show();
    LOG(INFO) << "Viewer backend: " << (host->backend() == Backend::Vulkan ? "Vulkan" : "OpenGL")
              << (host->fallback_reason_.empty() ? "" : "; Vulkan unavailable: " + host->fallback_reason_);
    return host;
}
void ViewerBase::poll_events() {
#if defined(EASY3D_HAS_VULKAN_VIEWER) || !defined(EASY3D_NO_OPENGL)
    detail::GlfwRuntime::poll_events();
#else
    throw BackendUnavailable("Interactive window support was not built");
#endif
}
bool ViewerBase::snapshot(const std::string& filename, float scaling, int sample_count, int background, bool expand) const {
    if (!std::isfinite(scaling) || scaling <= 0 || sample_count <= 0 || sample_count > 64 ||
        (sample_count & (sample_count - 1)) || background < 0 || background > 2 ||
        double(framebuffer_width()) * scaling > std::numeric_limits<int>::max() ||
        double(framebuffer_height()) * scaling > std::numeric_limits<int>::max()) return false;
    return snapshot_file(filename, scaling, sample_count, background, expand);
}
}
