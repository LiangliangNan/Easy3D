// SPDX-License-Identifier: GPL-3.0-or-later
#include "key_frame_interpolator.h"
#include "shape.h"
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>
#include <cmath>
#include <stdexcept>
namespace easy3d { namespace vulkan {
graphics::DrawCommand KeyFrameInterpolatorRenderer::draw_path(KeyFrameInterpolator& path,const Camera& camera,float thickness,const vec4& color) {
    const auto& samples=path.interpolate();
    if (samples.size()<2) return {};
    const auto mvp=camera.modelViewProjectionMatrix();
    bool same=path_command_.geometry && samples.size()==path_positions_.size() &&
              width_==camera.screenWidth() && height_==camera.screenHeight() && thickness_==thickness;
    for (size_t i=0;same && i<samples.size();++i) same=path_positions_[i]==samples[i].position();
    for (unsigned i=0;same && i<16;++i) same=path_mvp_[i]==mvp[i];
    if (!same) {
        path_positions_.clear();std::vector<vec3> segments;
        for (const auto& sample:samples) path_positions_.push_back(sample.position());
        for (size_t i=1;i<samples.size();++i) {segments.push_back(samples[i-1].position());segments.push_back(samples[i].position());}
        path_command_=shape::draw_lines_3d(segments,std::vector<vec3>(segments.size(),vec3(1)),mvp,
                                          camera.screenWidth(),camera.screenHeight(),thickness);
        width_=camera.screenWidth();height_=camera.screenHeight();thickness_=thickness;path_mvp_=mvp;
    }
    auto result=path_command_;result.color=color;return result;
}
graphics::DrawCommand KeyFrameInterpolatorRenderer::draw_cameras(KeyFrameInterpolator& path,const Camera& camera,float camera_width,const vec4& color) {
    if (!path.number_of_keyframes()) return {};
    if (!std::isfinite(camera_width) || camera_width<=0) throw std::invalid_argument("camera marker width must be positive and finite");
    const float aspect=float(camera.screenHeight())/camera.screenWidth();
    bool same=camera_command_.geometry && keyframes_.size()==path.number_of_keyframes() &&
              camera_width_==camera_width && fov_==camera.fieldOfView() && aspect_==aspect;
    for (size_t i=0;same && i<keyframes_.size();++i) {
        const auto frame=path.keyframe(i);
        same=keyframes_[i].position()==frame.position();
        for (unsigned j=0;same && j<4;++j) same=keyframes_[i].orientation()[j]==frame.orientation()[j];
    }
    if (!same) {
        keyframes_.clear();std::vector<graphics::Vertex> vertices;std::vector<uint32_t> indices;
        for (size_t i=0;i<path.number_of_keyframes();++i) {
            const auto frame=path.keyframe(i);keyframes_.push_back(frame);
            std::vector<vec3> points;std::vector<unsigned> elements;
            easy3d::shape::create_camera(points,elements,camera_width,camera.fieldOfView(),aspect);
            const auto offset=uint32_t(vertices.size());
            for (auto index:elements) indices.push_back(offset+index);
            const auto transform=frame.matrix();
            for (const auto& point:points) {graphics::Vertex v;v.position=transform*point;vertices.push_back(v);}
        }
        camera_command_.geometry=std::make_shared<graphics::Geometry>(std::move(vertices),std::move(indices));
        camera_command_.overlay=camera_command_.overlay_depth_test=true;
        camera_width_=camera_width;fov_=camera.fieldOfView();aspect_=aspect;
    }
    auto result=camera_command_;result.model_view_projection=camera.modelViewProjectionMatrix();result.color=color;return result;
}
} }
