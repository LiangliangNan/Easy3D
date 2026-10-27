// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TUTORIAL_IMAGE_VIEWER_VULKAN_H
#define EASY3D_TUTORIAL_IMAGE_VIEWER_VULKAN_H
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class TutorialImageViewer : public easy3d::VulkanViewer {
public:
    TutorialImageViewer(const std::string& title,const std::string& file,
                        const easy3d::VulkanViewerOptions& options = {}) : VulkanViewer(title,options) {
        if (!load_image(file)) throw std::runtime_error("could not load image: "+file);
        fit_screen();
    }
protected:
    bool key_press_event(int key,int modifiers) override {
        if (key == GLFW_KEY_F) { fit_screen(); return true; }
        if (key == GLFW_KEY_O && (modifiers & GLFW_MOD_CONTROL)) {
            const auto file = easy3d::dialog::open("Please choose an image file",easy3d::resource::directory()+"/data/",
                {"Image Files (*.png *.jpg *.bmp *.ppm *.tga)","*.png *.jpg *.bmp *.ppm *.tga"});
            if (!file.empty() && load_image(file)) { fit_screen(); return true; }
            return false;
        }
        return VulkanViewer::key_press_event(key,modifiers);
    }
    bool mouse_scroll_event(int,int,int,int dy) override {
        const float next = scale_*std::pow(1.1f,float(dy));
        scale_ = std::max(1e-6f,std::min(1e6f,next));
        dirty_ = true; update(); return true;
    }
    bool drop_event(const std::vector<std::string>& files) override {
        if (files.empty() || !load_image(files.front())) return false;
        fit_screen(); return true;
    }
    void post_resize(int,int) override { dirty_ = true; }
    void pre_draw() override {
        if (!dirty_) return;
        const float w = image_->width()*scale_, h = image_->height()*scale_;
        const float x = (width()-w)*0.5f, y = (height()-h)*0.5f;
        std::vector<easy3d::graphics::Vertex> vertices(4);
        vertices[0].position=easy3d::vec3(x,y,0); vertices[0].texcoord=easy3d::vec2(0,0);
        vertices[1].position=easy3d::vec3(x+w,y,0); vertices[1].texcoord=easy3d::vec2(1,0);
        vertices[2].position=easy3d::vec3(x+w,y+h,0); vertices[2].texcoord=easy3d::vec2(1,1);
        vertices[3].position=easy3d::vec3(x,y+h,0); vertices[3].texcoord=easy3d::vec2(0,1);
        easy3d::graphics::DrawCommand overlay;
        overlay.geometry=std::make_shared<easy3d::graphics::Geometry>(vertices,std::vector<uint32_t>{0,1,2,0,2,3});
        overlay.texture=image_;
        overlay.model_view_projection=easy3d::graphics::screen_space_projection(width(),height());
        set_overlays({overlay}); dirty_=false;
    }
    void fit_screen() {
        scale_=std::min(float(width())/image_->width(),float(height())/image_->height());
        dirty_=true; update();
    }
private:
    bool load_image(const std::string& file) {
        int w=0,h=0,channels=0;
        std::vector<unsigned char> rgba;
        if (!easy3d::ImageIO::load(file,rgba,w,h,channels,4,false)) return false;
        easy3d::graphics::Sampler sampler; sampler.filter=easy3d::graphics::TextureFilter::linear;
        auto image=std::make_shared<easy3d::graphics::Image>(w,h,std::move(rgba),sampler);
        image_=std::move(image); dirty_=true;
        return true;
    }
    std::shared_ptr<const easy3d::graphics::Image> image_;
    float scale_=1;
    bool dirty_=true;
};
#endif
