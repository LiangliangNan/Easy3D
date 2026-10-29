// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKEND_H
#define EASY3D_RENDERER_BACKEND_H
#include <stdexcept>
#include <string>
namespace easy3d {
enum class Backend { Automatic, Vulkan, OpenGL };
// Initialization cannot provide the requested backend/capabilities. Only this
// category permits Automatic to try another backend; rendering errors do not.
class BackendUnavailable : public std::runtime_error {
public:
    explicit BackendUnavailable(const std::string& message) : std::runtime_error(message) {}
};
}
#endif
