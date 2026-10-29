// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/backends/viewer.h>
#include <easy3d/viewer/viewer.h>
namespace easy3d { namespace detail {
std::unique_ptr<ViewerBase> create_opengl_viewer(const std::string& title, const ViewerOptions& o) {
    if (o.validation || o.synchronization_validation)
        throw BackendUnavailable("OpenGL viewer does not support the requested validation mode");
    std::unique_ptr<ViewerBase> host(new Viewer(title, o.samples == 1 ? 0 : int(o.samples), 3, 2,
                                               false, o.resizable, 24, 8, o.width, o.height));
    if (host->samples() != o.samples)
        throw BackendUnavailable("OpenGL viewer does not support the requested MSAA sample count");
    return host;
}
} }
