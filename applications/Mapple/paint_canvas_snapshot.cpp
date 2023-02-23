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


#define USE_QT_FBO
#define SHOW_PROGRESS

#ifdef  USE_QT_FBO
#include <QOpenGLFramebufferObject>
#else
#include <easy3d/renderer/framebuffer_object.h>
#endif

#include <QOpenGLFunctions>
#include <QMessageBox>
#include <QApplication>

#include "paint_canvas.h"
#include "main_window.h"
#include "walk_through.h"

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/core//model.h>
#include <easy3d/util/logging.h>

#ifdef SHOW_PROGRESS
#include <easy3d/util/progress.h>
#endif


using namespace easy3d;

bool PaintCanvas::saveSnapshot(int w, int h, int samples, const QString &file_name, int back_ground, bool expand) {
    int max_samples = 0;
    makeCurrent();
    func_->glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    doneCurrent();
    if (samples > max_samples) {
        LOG(WARNING) << "requested samples (" << samples << ") exceeds the supported maximum samples (" << max_samples
                     << ")";
        return false;
    }

    int sub_w = static_cast<int>(static_cast<float>(width()) * dpi_scaling());
    int sub_h = static_cast<int>(static_cast<float>(height()) * dpi_scaling());

    double aspectRatio = sub_w / static_cast<double>(sub_h);
    double newAspectRatio = w / static_cast<double>(h);
    double zNear = camera()->zNear();
    double zFar = camera()->zFar();

    float xMin = 0.0f, yMin = 0.0f;
    if (camera()->type() == Camera::PERSPECTIVE)
        if ((expand && (newAspectRatio > aspectRatio)) || (!expand && (newAspectRatio < aspectRatio))) {
            yMin = static_cast<float>(zNear * std::tan(camera()->fieldOfView() / 2.0));
            xMin = static_cast<float>(newAspectRatio * yMin);
        } else {
            xMin = static_cast<float>(zNear * std::tan(camera()->fieldOfView() / 2.0) * aspectRatio);
            yMin = static_cast<float>(xMin / newAspectRatio);
        }
    else {
        camera()->getOrthoWidthHeight(xMin, yMin);
        if ((expand && (newAspectRatio > aspectRatio)) || (!expand && (newAspectRatio < aspectRatio)))
            xMin = static_cast<float>(newAspectRatio * yMin);
        else
            yMin = static_cast<float>(xMin / newAspectRatio);
    }

    QImage image(w, h, QImage::Format_RGBA8888);
    if (image.isNull()) {
        QMessageBox::warning(this, "Image saving error", "Failed to allocate the image", QMessageBox::Ok,
                             QMessageBox::NoButton);
        doneCurrent();
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
    // const mat4& proj_matrix = camera()->projectionMatrix(); // Liangliang: This will definitely NOT work !!!
    const mat4 proj_matrix = camera()->projectionMatrix();

    // temporarily don't allow updating rendering when the camera parameters are changing.
    easy3d::disconnect_all(&camera_->frame_modified);

    makeCurrent();

#ifdef USE_QT_FBO
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(samples);
    auto fbo = new QOpenGLFramebufferObject(sub_w, sub_h, format);
    fbo->addColorAttachment(sub_w, sub_h);
#else
    auto fbo = new FramebufferObject(sub_w, sub_h, samples);
    fbo->add_color_buffer();
    fbo->add_depth_buffer();
#endif

    int count = 0;
#ifdef SHOW_PROGRESS
    ProgressLogger progress(static_cast<std::size_t>(nbX * nbY * 1.1), false, false); // the extra 0.1 is for saving the "big" image (time-consuming)
#endif
    for (int i = 0; i < nbX; i++) {
        for (int j = 0; j < nbY; j++) {
#ifdef SHOW_PROGRESS
            if (progress.is_canceled()) {
                LOG(WARNING) << "snapshot cancelled";
                break;
            }
#endif
            if (camera()->type() == Camera::PERSPECTIVE) {
                // change the projection matrix of the camera.
                const mat4 &proj = transform::frustum(
                        static_cast<float>(-xMin + i * deltaX),
                        static_cast<float>(-xMin + (i + 1) * deltaX),
                        static_cast<float>(yMin - (j + 1) * deltaY),
                        static_cast<float>(yMin - j * deltaY),
                        static_cast<float>(zNear),
                        static_cast<float>(zFar)
                );
                camera()->set_projection_matrix(proj);
            } else {
                // change the projection matrix of the camera.
                const mat4 &proj = transform::ortho(
                        static_cast<float>(-xMin + i * deltaX),
                        static_cast<float>(-xMin + (i + 1) * deltaX),
                        static_cast<float>(yMin - (j + 1) * deltaY),
                        static_cast<float>(yMin - j * deltaY),
                        static_cast<float>(zNear),
                        static_cast<float>(zFar)
                );
                camera()->set_projection_matrix(proj);
            }

            //---------------------------------------------------------------------------

            // this very important (the progress bar may interfere the framebuffer)
            makeCurrent();

            fbo->bind();

            // 0: current color; 1: white; 2: transparent.
            if (back_ground == 1)       // white
                func_->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            else if (back_ground == 2)  // transparent
                func_->glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
            else                        // current color
                func_->glClearColor(background_color_[0], background_color_[1], background_color_[2],
                                    background_color_[3]);
            func_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            draw();

            fbo->release();

            //---------------------------------------------------------------------------


#ifdef USE_QT_FBO
            const QImage subImage = fbo->toImage();
#else
            QImage subImage(sub_w, sub_h, QImage::Format_RGBA8888);
            fbo->read_color(0, const_cast<unsigned char*>(subImage.constBits()), GL_RGBA);
#endif

            // Copy subImage in image
            for (int ii = 0; ii < sub_w; ii++) {
                int fi = i * sub_w + ii;
                if (fi == image.width())
                    break;
                for (int jj = 0; jj < sub_h; jj++) {
                    int fj = j * sub_h + jj;
                    if (fj == image.height())
                        break;
                    image.setPixel(fi, fj, subImage.pixel(ii, jj));
                }
            }
            /************************************************************************/
            // save each tile (mainly for debug purpose)
            //QString name = QString("tmp-%1.png").arg(count);
            //subImage.save(name);
            //std::cout << "tile: " << count << "/" << nbX * nbY << std::endl;
            /************************************************************************/
            ++count;

#ifdef SHOW_PROGRESS
            progress.next();
#endif
        }
    }

    // this very important (the progress bar may interfere the framebuffer)
    makeCurrent();
    // clean
    delete fbo;
    // restore the clear color
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
    doneCurrent();

    // restore the projection matrix
    camera()->set_projection_matrix(proj_matrix);

    // enable updating the rendering
    easy3d::connect(&camera_->frame_modified, this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));

    return image.save(file_name);
}


#ifdef HAS_FFMPEG

#include <easy3d/video/video_encoder.h>

void PaintCanvas::recordAnimation(const QString &file_name, int fps, int bit_rate, bool bk_white) {
    auto kfi = walkThrough()->interpolator();
    if (!kfi || kfi->number_of_keyframes() == 0) {
        LOG_IF(kfi->number_of_keyframes() == 0, WARNING)
                        << "nothing to record (camera path is empty). You may import a camera path from a file or"
                           " creat it by adding keyframes";
        return;
    }

    // make the image dimensions multiples of 8 (to have the same size as if video saved using ffmpeg)
    const int original_width = width();
    const int original_height = height();
    int w = original_width;
    int h = original_height;
    //find the nearest multiples of 8
    if (original_width % 8)
        w = (original_width / 8 + 1) * 8;
    if (original_height % 8)
        h = (original_height / 8 + 1) * 8;
    if (w != original_width || h != original_height)
        camera_->setScreenWidthAndHeight(w, h);

    // temporarily don't allow updating rendering when the camera parameters are changing.
    easy3d::disconnect_all(&camera_->frame_modified);

    const int bitrate = bit_rate * 1024 * 1024;
    bool success = true;

    const int fw = static_cast<int>(static_cast<float>(w) * dpi_scaling());
    const int fh = static_cast<int>(static_cast<float>(h) * dpi_scaling());
    VideoEncoder encoder;
    if (!encoder.start(file_name.toStdString(), fps, bitrate)) {
        // clean up and restore the settings before exit
        encoder.end();
        setEnabled(true);
        easy3d::connect(&camera_->frame_modified, this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));
        return;
    }

    const auto &frames = kfi->interpolate();
    makeCurrent();

#ifdef USE_QT_FBO
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(samples());
    auto fbo = new QOpenGLFramebufferObject(fw, fh, format);
    fbo->addColorAttachment(fw, fh, GL_RGBA);
#else
    auto fbo = new FramebufferObject(fw, fh, samples());
    fbo->add_color_buffer();
    fbo->add_depth_buffer();
#endif

#ifdef SHOW_PROGRESS
    ProgressLogger progress(frames.size(), true, false);
#endif
    for (const auto& f : frames) {
#ifdef SHOW_PROGRESS
        if (progress.is_canceled()) {
            success = false;
            LOG(WARNING) << "animation recording cancelled";
            break;
        }
#endif

        camera_->setPosition(f.position());
        camera_->setOrientation(f.orientation());

        // this very important (the progress bar may interfere the framebuffer)
        makeCurrent();

        fbo->bind();

        if (bk_white)
            func_->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        else
            func_->glClearColor(background_color_[0], background_color_[1], background_color_[2],
                                background_color_[3]);
        func_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        draw();

        fbo->release();

        //---------------------------------------------------------------------------

#ifdef USE_QT_FBO
        QImage image = fbo->toImage();
        if (image.isNull()) {
            LOG(ERROR) << "failed to grab the framebuffer";
            success = false;
            break;
        }
        if (image.format() != QImage::Format_RGBA8888)
            image = image.convertToFormat(QImage::Format_RGBA8888);
        if (!encoder.encode(image.constBits(), image.width(), image.height(), VideoEncoder::PIX_FMT_RGBA_8888)) {
            success = false;
            break;
        }
#else
        std::vector<unsigned char> data;
        fbo->read_color(0, data, GL_RGBA);
        if (!encoder.encode(data.data(), fw, fh, VideoEncoder::PIX_FMT_RGBA_8888)) {
            success = false;
            break;
        }
#endif

#ifdef SHOW_PROGRESS
        progress.next();
#endif
    }

    // this very important (the progress bar may interfere the framebuffer)
    makeCurrent();
    // clean
    delete fbo;
    // restore the clear color
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
    doneCurrent();

    encoder.end();

    // enable updating the rendering
    easy3d::connect(&camera_->frame_modified, this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));  // this works
//    easy3d::connect(&camera_->frame_modified, this, overload<PaintCanvas>(&PaintCanvas::update));  // this also works
//    camera_->frame_modified.connect(this, static_cast<void (PaintCanvas::*)(void)> (&PaintCanvas::update));  // this also works
//    camera_->frame_modified.connect(this, overload<PaintCanvas>(&PaintCanvas::update));  // this also works
//    camera_->frame_modified.connect(this, &PaintCanvas::update);  // this won't work due to overloading
//    easy3d::connect(&camera_->frame_modified, this, &PaintCanvas::update);  // this won't work due to overloading

    // restore the viewer size
    if (w != original_width || h != original_height) {
        camera_->setScreenWidthAndHeight(original_width, original_height);
        update();
    }

    if (success)
        LOG(INFO) << "animation has been saved successfully";
    else
        LOG(ERROR) << "animation recording failed";
}

#else

void PaintCanvas::recordAnimation(const QString &file_name, int, int, bool bk_white) {
    auto kfi = walkThrough()->interpolator();
    if (kfi->number_of_keyframes() == 0) {
        LOG(WARNING) << "recording aborted (camera path is empty). You may import a camera path from a file or"
                        " creat it by adding key frames";
        return;
    }

    // make the image dimensions multiples of 8 (to have the same size as if video saved using ffmpeg)
    const int original_width = width();
    const int original_height = height();
    int w = original_width;
    int h = original_height;
    //find the nearest multiples of 8
    if (original_width % 8)
        w = (original_width / 8 + 1) * 8;
    if (original_height % 8)
        h = (original_height / 8 + 1) * 8;
    if (w != original_width || h != original_height)
        camera_->setScreenWidthAndHeight(w, h);

    // temporarily don't allow updating rendering when the camera parameters are changing.
    easy3d::disconnect_all(&camera_->frame_modified);

    const int fw = w * dpi_scaling();
    const int fh = h * dpi_scaling();
    const auto &frames = kfi->interpolate();
    makeCurrent();

#ifdef USE_QT_FBO
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(samples());
    QOpenGLFramebufferObject *fbo = new QOpenGLFramebufferObject(fw, fh, format);
    fbo->addColorAttachment(fw, fh);
#else
    FramebufferObject* fbo = new FramebufferObject(fw, fh, samples());
    fbo->add_color_buffer();
    fbo->add_depth_buffer();
#endif

    bool success = true;
    const QString ext_less_name = file_name.left(file_name.lastIndexOf('.'));
#ifdef SHOW_PROGRESS
    ProgressLogger progress(frames.size(), true, false);
#endif
    for (std::size_t frame_index = 0; frame_index < frames.size(); ++frame_index) {
#ifdef SHOW_PROGRESS
        if (progress.is_canceled()) {
            success = false;
            LOG(WARNING) << "animation recording cancelled";
            break;
        }
#endif

        const auto &f = frames[frame_index];
        camera_->setPosition(f.position());
        camera_->setOrientation(f.orientation());

        // this very important (the progress bar may interfere the framebuffer)
        makeCurrent();

        fbo->bind();

        if (bk_white)
            func_->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        else
            func_->glClearColor(background_color_[0], background_color_[1], background_color_[2],
                                background_color_[3]);
        func_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        draw();

        fbo->release();

        //---------------------------------------------------------------------------

#ifdef USE_QT_FBO
        const QImage image = fbo->toImage();
        if (image.isNull()) {
            QMessageBox::critical(this, "Error", "Failed to grab the framebuffer!");
            success = false;
            break;
        }
#else
        QImage image(fw, fh, QImage::Format_RGBA8888);
        fbo->read_color(0, const_cast<unsigned char*>(image.constBits()), GL_RGBA);
#endif

        const QString full_name = ext_less_name + QString("-%1.png").arg(frame_index, 4, 10, QChar('0'));
        if (!image.save(full_name)) {
            QMessageBox::critical(this, "Error", QString("failed to save the %1-th frame").arg(frame_index));
            success = false;
            break;
        }

#ifdef SHOW_PROGRESS
        progress.next();
#endif
    }

    // this very important (the progress bar may interfere the framebuffer)
    makeCurrent();
    // clean
    delete fbo;
    // restore the clear color
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
    doneCurrent();

    // enable updating the rendering
    easy3d::connect(&camera_->frame_modified, this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));

    // restore the viewer size
    if (w != original_width || h != original_height) {
        camera_->setScreenWidthAndHeight(original_width, original_height);
        update();
    }

    if (success)
        LOG(INFO) << "animation (in " << frames.size() << " images) have been saved successfully";
    else
        LOG(ERROR) << "animation recording failed";
}


#endif
