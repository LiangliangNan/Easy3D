// SPDX-License-Identifier: GPL-3.0-or-later
#include "manipulator.h"
#include "shape.h"
#include <easy3d/core/model.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/camera.h>
namespace easy3d { namespace vulkan {
graphics::DrawCommand ManipulatorRenderer::draw_frame(const Model& model,const Camera& camera,float line_width) {
    if (!model.manipulator() || !model.bounding_box().is_valid()) return {};
    const auto box=model.bounding_box();
    const auto transform=model.manipulator()->matrix()*mat4::translation(box.center())*
                         mat4::scale(box.range(0),box.range(1),box.range(2),1.0f);
    const auto mvp=camera.modelViewProjectionMatrix()*transform;
    bool same=command_.geometry && width_==camera.screenWidth() && height_==camera.screenHeight() && line_width_==line_width;
    for (unsigned i=0;same && i<16;++i) same=mvp_[i]==mvp[i];
    if (!same) {
        command_=shape::draw_box_wire(mvp,camera.screenWidth(),camera.screenHeight(),line_width,true);
        mvp_=mvp;width_=camera.screenWidth();height_=camera.screenHeight();line_width_=line_width;
    }
    return command_;
}
} }
