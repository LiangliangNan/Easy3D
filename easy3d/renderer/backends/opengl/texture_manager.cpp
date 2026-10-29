// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/texture_manager.h>
namespace easy3d {
// Called with the last viewer share-group context current. CPU-only textures
// may still be referenced by Vulkan hosts and must outlive OpenGL shutdown.
void TextureManager::terminate_opengl() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = textures_.begin(); it != textures_.end();) {
        if (it->second->id()) {
            delete it->second;
            it = textures_.erase(it);
        } else ++it;
    }
    for (auto it = attempt_load_texture_.begin(); it != attempt_load_texture_.end();) {
        if (it->first.compare(0, 4, "cpu|") != 0) it = attempt_load_texture_.erase(it);
        else ++it;
    }
}
}
