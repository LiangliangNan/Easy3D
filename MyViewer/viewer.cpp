#include "viewer.h"
#include "drawable_points.h"
#include "drawable_lines.h"
#include "drawable_triangles.h"

#include <lve_renderer.hpp>
#include <vk_ui_overlay.h>
#include <camera.hpp>


namespace lve {


    Viewer::Viewer(const std::string &name) : VulkanApplication(name, false) {
    }


    Viewer::~Viewer() {
        drawables_.clear();
    }


    PointsDrawable& Viewer::createPointsDrawable() {
        auto drawable = std::make_unique<PointsDrawable>(*lveDevice, *descriptorAllocator, lveRenderer->getSwapChainRenderPass());
        drawables_.push_back(std::move(drawable));
        viewChanged();
        return *dynamic_cast<PointsDrawable*>(drawables_.back().get());
    }

    LinesDrawable& Viewer::createLinesDrawable() {
        auto drawable = std::make_unique<LinesDrawable>(*lveDevice, *descriptorAllocator, lveRenderer->getSwapChainRenderPass());
        drawables_.push_back(std::move(drawable));
        viewChanged();
        return *dynamic_cast<LinesDrawable*>(drawables_.back().get());
    }

    TrianglesDrawable& Viewer::createTrianglesDrawable() {
        auto drawable = std::make_unique<TrianglesDrawable>(*lveDevice, *descriptorAllocator, lveRenderer->getSwapChainRenderPass());
        drawables_.push_back(std::move(drawable));
        viewChanged();
        return *dynamic_cast<TrianglesDrawable*>(drawables_.back().get());
    }


    void Viewer::viewChanged() {
        // todo: update only when the view has changed
        for (auto &d: drawables_)
            d->updateUniformBuffer(camera_->matrices.perspective, camera_->matrices.view);
    }


    void Viewer::draw(VkCommandBuffer cmdBuffer) {
        for (auto &d: drawables_)
            d->draw(cmdBuffer);
    }

    void Viewer::fillUI() {
        if (uiOverlay->header("Settings")) {
            for (auto &d: drawables_) {
                bool visible = d->is_visible();
                if (uiOverlay->checkBox(d->name().c_str(), &visible))
                    d->set_visible(visible);
            }

//            if (vulkanDevice->features.fillModeNonSolid) {
//                if (uiOverlay->checkBox("Splitscreen", &splitScreen)) {
//                    buildCommandBuffers();
//                }
//            }
        }
    }

    void Viewer::onKeyEvent(const KeyEvent& event) {
//        if (settings.overlay && UIOverlay_->on_event(event))
//            return; // the event has already captured and processed.
        if (event.get_action() == KeyAction::Down) {
            Key key = event.get_code();
            auto pos_func = commands_.find(key);
            if (pos_func != commands_.end()) {
                auto func = pos_func->second;
                func(this);
                return;
            }
        }
        VulkanApplication::onKeyEvent(event);
    }

//    void Viewer::onUpdateUIOverlay() {
//        if (UIOverlay_->header("Settings")) {
//            for (auto &d: drawables_) {
//                bool visible = d->is_visible();
//                if (UIOverlay_->checkBox(d->name().c_str(), &visible))
//                    d->set_visible(visible);
//            }
//
//            if (vulkanDevice->features.fillModeNonSolid) {
//                if (UIOverlay_->checkBox("Splitscreen", &splitScreen)) {
//                    buildCommandBuffers();
//                }
//            }
//        }
//    }

}