// SPDX-License-Identifier: GPL-3.0-or-later
// Temporary bridge for the legacy OffScreen facade; removed in migration step 4.
#include <easy3d/viewer/backends/offscreen.h>
#include <easy3d/viewer/opengl_offscreen.h>
namespace easy3d { namespace detail {
namespace {
class OpenGLOffScreenHost : public OpenGLOffScreen {
public:
    OpenGLOffScreenHost(int w,int h,const OffScreenOptions&) : OpenGLOffScreen(w,h) {}
    void activate() const {} // concrete operations manage their own context
};
}
std::unique_ptr<OffScreenBackend> create_opengl_offscreen(int w,int h,const OffScreenOptions& o) {
    if(o.validation || o.synchronization_validation)
        throw BackendUnavailable("OpenGL offscreen does not support the requested validation mode");
    return std::unique_ptr<OffScreenBackend>(new OffScreenAdapter<OpenGLOffScreenHost>(w,h,o));
}
} }
