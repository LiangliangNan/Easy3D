// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TUTORIAL_VIRTUAL_SCANNER_VULKAN_H
#define EASY3D_TUTORIAL_VIRTUAL_SCANNER_VULKAN_H
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/algo/gaussian_noise.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>

class TutorialVirtualScanner : public easy3d::VulkanViewer {
public:
    explicit TutorialVirtualScanner(const std::string& title="VirtualScanner",
            const easy3d::VulkanViewerOptions& options={}) : VulkanViewer(title,options) {
        camera()->setUpVector(easy3d::vec3(0,1,0));
    }
protected:
    bool key_press_event(int key,int modifiers) override {
        if (modifiers==0 && key==GLFW_KEY_N) { add_noise_=!add_noise_; return true; }
        if (modifiers==0 && key==GLFW_KEY_SPACE) {
            int w,h; framebuffer_size(w,h);
            if (w<=0 || h<=0) return true;
            // Key events may arrive before the next draw synchronizes a resize.
            if (width()!=w || height()!=h) easy3d::VulkanOffScreen::resize(w,h);
            const auto points=scan_visible_points(samples());
            if (points.empty()) return true;
            auto cloud=std::make_shared<easy3d::PointCloud>();
            cloud->set_name("Virtual scan");
            for (const auto& p:points) cloud->add_vertex(p);
            if (add_noise_) easy3d::GaussianNoise::apply(cloud.get(),camera()->sceneRadius()*.0001f);
            // Keep the complete acquisition camera state, including its pivot.
            add_model(cloud,true,false);
            update(); return true;
        }
        return VulkanViewer::key_press_event(key,modifiers);
    }
private:
    bool add_noise_=false;
};
#endif
