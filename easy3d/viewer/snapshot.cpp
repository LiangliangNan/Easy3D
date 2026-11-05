/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/core/matrix.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/logging.h>
#include <cmath>
#include <limits>
#include <stdexcept>
#ifdef EASY3D_HAS_VULKAN
namespace easy3d {
// The device has no framebuffer to bind, so the snapshot renders a camera copy
// offscreen. This mirrors the OpenGL path's camera/projection setup, but uses
// graphics::RenderDevice::render() instead of an OpenGL framebuffer object.
bool Viewer::snapshot(const std::string& filename, float scaling, int samples,
                      int back_ground, bool expand) const {
    if (!initialized_) {
        auto* host = const_cast<Viewer*>(this); host->init(); host->initialized_ = true;
    }
    if (window_) { update_camera_path_overlays(); update_interaction_overlays(); }
    try {
        if (samples <= 0) throw std::invalid_argument("sample count must be positive");
        if (!std::isfinite(scaling) || scaling <= 0 || double(framebuffer_width()) * scaling > std::numeric_limits<int>::max() ||
            double(framebuffer_height()) * scaling > std::numeric_limits<int>::max() || back_ground < 0 || back_ground > 2)
            throw std::invalid_argument("invalid snapshot scaling/background");
        Camera copy(*camera());
        const int w = int(std::ceil(framebuffer_width() * double(scaling))), h = int(std::ceil(framebuffer_height() * double(scaling)));
        if (w <= 0 || h <= 0) throw std::invalid_argument("scaled snapshot dimensions are zero");
        copy.setScreenWidthAndHeight(w, h);
        copy.projectionMatrix(); // resolve resize invalidation before overriding the matrix
        // Match the OpenGL snapshot: preserve one source frustum dimension,
        // expanding or cropping the other for the rounded output aspect ratio.
        const double aspect = double(framebuffer_width()) / framebuffer_height(), output_aspect = double(w) / h;
        const double near = camera()->zNear(), far = camera()->zFar();
        float x, y;
        if (camera()->type() == Camera::PERSPECTIVE) {
            y = float(near * std::tan(camera()->fieldOfView() * .5));
            x = float(aspect * y);
        } else camera()->getOrthoWidthHeight(x, y);
        if ((expand && output_aspect > aspect) || (!expand && output_aspect < aspect))
            x = float(output_aspect * y);
        else y = float(x / output_aspect);
        copy.set_projection_matrix(camera()->type() == Camera::PERSPECTIVE
            ? transform::frustum(-x, x, -y, y, float(near), float(far))
            : transform::ortho(-x, x, -y, y, float(near), float(far)));
        const vec4 background = back_ground == 0 ? background_color() : back_ground == 1 ? vec4(1, 1, 1, 1) : vec4(0, 0, 0, 0);
        auto frame = const_cast<graphics::RenderDevice&>(device()).render(
            uint32_t(w), uint32_t(h), frame_commands(copy, w, h, nullptr, snapshot_hud_),
            background, unsigned(samples));
        if (back_ground == 2) {
            // Alpha overlays and MSAA coverage against transparent black produce
            // premultiplied colors, including single-sample text. PNG stores straight alpha.
            for (size_t i = 0; i < frame.rgba.size(); i += 4) {
                const unsigned alpha = frame.rgba[i + 3];
                if (alpha && alpha < 255)
                    for (unsigned c = 0; c < 3; ++c)
                        frame.rgba[i + c] = static_cast<unsigned char>(std::min(255u,
                            (unsigned(frame.rgba[i + c]) * 255 + alpha / 2) / alpha));
            }
        }
        return ImageIO::save(filename, frame.rgba, w, h, 4, false);
    } catch (const std::exception& e) {
        LOG(ERROR) << "Vulkan snapshot failed: " << e.what(); return false;
    }
}
}
#else
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <easy3d/util/logging.h>
namespace easy3d {


#if 0 // Very basic snapshot: image size is the same as the viewer size

	bool Viewer::snapshot(const std::string& file_name, bool bk_white) const {
		int w, h;
		framebuffer_size(w, h);

		FramebufferObject fbo(w, h, samples_);
		fbo.add_color_buffer();
		fbo.add_depth_buffer();

		fbo.bind();

		if (bk_white)
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		else
			glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		const_cast<Viewer*>(this)->draw();

		fbo.release();

#if 1   // color render buffer
		return fbo.snapshot_color(0, file_name);
#else
		// depth buffer
		return fbo.snapshot_depth(file_name);
#endif
	}

#else // Advanced snapshot: image size, samples, and background can be customized.

	bool Viewer::snapshot(const std::string& file_name, float scaling, int samples, int back_ground, bool expand) const {
        if (samples == 1) samples = 0;
        glfwMakeContextCurrent(window_);
        if (!initialized_) {
            auto* host = const_cast<Viewer*>(this); host->init(); host->initialized_ = true;
        }
		int max_samples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
		if (samples > max_samples) {
			LOG(WARNING) << "requested samples (" << samples << ") exceeds the supported maximum samples (" << max_samples
				<< ")";
			return false;
		}

		int w, h;
		framebuffer_size(w, h);
		w = static_cast<int>(std::ceil(static_cast<float>(w) * scaling));
		h = static_cast<int>(std::ceil(static_cast<float>(h) * scaling));

		int sub_w = static_cast<int>(static_cast<float>(width()) * dpi_scaling());
		int sub_h = static_cast<int>(static_cast<float>(height()) * dpi_scaling());

		double aspectRatio = sub_w / static_cast<double>(sub_h);
		double newAspectRatio = w / static_cast<double>(h);
		double zNear = camera_->zNear();
		double zFar = camera_->zFar();

		float xMin = 0.0f, yMin = 0.0f;
		if (camera_->type() == Camera::PERSPECTIVE)
			if ((expand && (newAspectRatio > aspectRatio)) || (!expand && (newAspectRatio < aspectRatio))) {
				yMin = static_cast<float>(zNear * std::tan(camera_->fieldOfView() / 2.0));
				xMin = static_cast<float>(newAspectRatio * yMin);
			}
			else {
				xMin = static_cast<float>(zNear * std::tan(camera_->fieldOfView() / 2.0) * aspectRatio);
				yMin = static_cast<float>(xMin / newAspectRatio);
			}
		else {
			camera_->getOrthoWidthHeight(xMin, yMin);
			if ((expand && (newAspectRatio > aspectRatio)) || (!expand && (newAspectRatio < aspectRatio)))
				xMin = static_cast<float>(newAspectRatio * yMin);
			else
				yMin = static_cast<float>(xMin / newAspectRatio);
		}

		typedef Vec<4, unsigned char> Pixel; // in RGBA order
		Matrix<Pixel> image(h, w); // Note: rows first
		if (!image.data()) {
			LOG(ERROR) << "failed to allocate the image with size " << w << " x " << h;
			return false;
		}

		double scaleX = sub_w / static_cast<double>(w);
		double scaleY = sub_h / static_cast<double>(h);
		double deltaX = 2.0 * xMin * scaleX;
		double deltaY = 2.0 * yMin * scaleY;
		int nbX = w / sub_w;
		int nbY = h / sub_h;
		// Extra subimage on the right/bottom border(s) if needed
		if (nbX * sub_w < w) ++nbX;
		if (nbY * sub_h < h) ++nbY;

		// remember the current projection matrix
		// const mat4& proj_matrix = camera_->projectionMatrix(); // Liangliang: This will definitely NOT work !!!
		const mat4 proj_matrix = camera_->projectionMatrix();

		// temporarily don't allow updating rendering when the camera parameters are changing.
		easy3d::disconnect_all(&camera_->frame_modified);

		auto fbo = new FramebufferObject(sub_w, sub_h, samples);
		fbo->add_color_buffer();
		fbo->add_depth_buffer();

		for (int i = 0; i < nbX; i++) {
			for (int j = 0; j < nbY; j++) {
				if (camera_->type() == Camera::PERSPECTIVE) {
					// change the projection matrix of the camera.
					const mat4& proj = transform::frustum(
						static_cast<float>(-xMin + i * deltaX),
						static_cast<float>(-xMin + (i + 1) * deltaX),
						static_cast<float>(yMin - (j + 1) * deltaY),
						static_cast<float>(yMin - j * deltaY),
						static_cast<float>(zNear),
						static_cast<float>(zFar)
					);
					camera_->set_projection_matrix(proj);
				}
				else {
					// change the projection matrix of the camera.
					const mat4& proj = transform::ortho(
						static_cast<float>(-xMin + i * deltaX),
						static_cast<float>(-xMin + (i + 1) * deltaX),
						static_cast<float>(yMin - (j + 1) * deltaY),
						static_cast<float>(yMin - j * deltaY),
						static_cast<float>(zNear),
						static_cast<float>(zFar)
					);
					camera_->set_projection_matrix(proj);
				}

				//---------------------------------------------------------------------------

				fbo->bind();

				// 0: current color; 1: white; 2: transparent.
				if (back_ground == 1)       // white
					glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				else if (back_ground == 2)  // transparent
					glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				else                        // current color
					glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                struct TileReset { ivec4& value;ivec4 old;~TileReset(){value=old;} } reset{overlay_tile_,overlay_tile_};
                overlay_tile_=ivec4(i*sub_w,j*sub_h,w,h);
				const_cast<Viewer*>(this)->draw();
                // Scene annotations belong in snapshots; HUD callbacks do not.
                if (show_camera_path_ && !kfi_->is_interpolation_started()) {
                    kfi_->draw_path(camera(), 2.0f);
                    kfi_->draw_cameras(camera(), camera()->sceneRadius() * 0.05f);
                }
                draw_overlays(true);

				fbo->release();

				//---------------------------------------------------------------------------

				Matrix<Pixel> subImage(sub_h, sub_w); // Note: rows first
				fbo->read_color(0, reinterpret_cast<unsigned char*>(subImage.data()), GL_RGBA);

				// Copy subImage in image
				for (int ii = 0; ii < sub_w; ii++) {
					int fi = i * sub_w + ii;
					if (fi == image.cols())
						break;
					for (int jj = 0; jj < sub_h; jj++) {
						int fj = j * sub_h + jj;
						if (fj == image.rows())
							break;
						image.set(fj, fi, subImage.get(jj, ii));
					}
				}
			}
		}

		// clean
		delete fbo;
		// restore the clear color
		glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

		// restore the projection matrix
		camera_->set_projection_matrix(proj_matrix);

		// enable updating the rendering
		easy3d::connect(&camera_->frame_modified, const_cast<Viewer*>(this), &Viewer::update);

		Pixel* pixels = image.data();
		std::vector<unsigned char> data(reinterpret_cast<unsigned char*>(pixels), reinterpret_cast<unsigned char*>(pixels) + image.rows()* image.cols() * sizeof(Pixel));

        // Alpha compositing over transparent black stores premultiplied RGB.
        // ImageIO expects straight-alpha pixels, as in MultiViewer snapshots.
        if(back_ground==2) for(size_t i=0;i<data.size();i+=4) if(data[i+3] && data[i+3]<255)
            for(unsigned c=0;c<3;++c)
                data[i+c]=static_cast<unsigned char>(std::min(255u,(unsigned(data[i+c])*255+data[i+3]/2)/data[i+3]));
		return ImageIO::save(file_name, data, image.cols(), image.rows(), 4);
	}

}


#endif

#endif
