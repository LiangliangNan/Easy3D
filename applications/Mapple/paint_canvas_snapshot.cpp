/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#define USE_QT_FBO
#define SHOW_PROGRESS

#ifdef  USE_QT_FBO
#include <QOpenGLFramebufferObject>
#else
#include <easy3d/renderer/framebuffer_object.h>
#endif

#include <QOpenGLFunctions>
#include <QMessageBox>

#include "paint_canvas.h"
#include "main_window.h"

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/core//model.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/progress.h>


using namespace easy3d;

bool PaintCanvas::saveSnapshot(int w, int h, int samples, const QString &file_name, bool bk_white, bool expand)
{
    int max_samples = 0;
    makeCurrent();
    func_->glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    doneCurrent();
    if (samples > max_samples) {
        LOG(WARNING) << "requested samples (" << samples << ") exceeds the supported maximum samples (" << max_samples << ")";
        return false;
    }

    int sub_w = static_cast<int>(width() * dpi_scaling());
    int sub_h = static_cast<int>(height() * dpi_scaling());

    double aspectRatio = sub_w / static_cast<double>(sub_h);
    double newAspectRatio = w / static_cast<double>(h);
    double zNear = camera()->zNear();
    double zFar = camera()->zFar();

    float xMin = 0.0f, yMin = 0.0f;
    if (camera()->type() == Camera::PERSPECTIVE)
        if ((expand && (newAspectRatio>aspectRatio)) || (!expand && (newAspectRatio<aspectRatio))) {
            yMin = zNear * tan(camera()->fieldOfView() / 2.0);
            xMin = newAspectRatio * yMin;
        }
        else {
            xMin = zNear * tan(camera()->fieldOfView() / 2.0) * aspectRatio;
            yMin = xMin / newAspectRatio;
        }
    else {
        camera()->getOrthoWidthHeight(xMin, yMin);
        if ((expand && (newAspectRatio>aspectRatio)) || (!expand && (newAspectRatio<aspectRatio)))
            xMin = newAspectRatio * yMin;
        else
            yMin = xMin / newAspectRatio;
    }

    QImage image(w, h, QImage::Format_ARGB32);
    if (image.isNull()) {
        QMessageBox::warning(this, "Image saving error", "Failed to allocate the image", QMessageBox::Ok, QMessageBox::NoButton);
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
    if (nbX * sub_w < w)	++nbX;
    if (nbY * sub_h < h)	++nbY;

    ProgressLogger progress(nbX*nbY * 1.2f); // the extra 0.2 is for saving the "big" image

    // remember the current projection matrix
    // const mat4& proj_matrix = camera()->projectionMatrix(); // Liangliang: This will definitely NOT work !!!
    const mat4 proj_matrix = camera()->projectionMatrix();

    makeCurrent();

#ifdef USE_QT_FBO
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(samples);
    QOpenGLFramebufferObject* fbo = new QOpenGLFramebufferObject(sub_w, sub_h, format);
    fbo->addColorAttachment(sub_w, sub_h);
#else
    FramebufferObject* fbo = new FramebufferObject(sub_w, sub_h, samples);
    fbo->add_color_buffer();
    fbo->add_depth_buffer();
#endif

    int count = 0;
    for (int i = 0; i < nbX; i++) {
        for (int j = 0; j < nbY; j++) {
            if (camera()->type() == Camera::PERSPECTIVE) {
                // change the projection matrix of the camera.
                const mat4& proj = transform::frustum(-xMin + i*deltaX, -xMin + (i + 1)*deltaX, yMin - (j + 1)*deltaY, yMin - j*deltaY, zNear, zFar);
                camera()->set_projection_matrix(proj);
            }
            else {
                // change the projection matrix of the camera.
                const mat4& proj = transform::ortho(-xMin + i*deltaX, -xMin + (i + 1)*deltaX, yMin - (j + 1)*deltaY, yMin - j*deltaY, zNear, zFar);
                camera()->set_projection_matrix(proj);
            }

            //---------------------------------------------------------------------------

            fbo->bind();

			if (bk_white)
				func_->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			else
				func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
            func_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            draw();

            fbo->release();

            //---------------------------------------------------------------------------

#ifdef USE_QT_FBO
            const QImage subImage = fbo->toImage();
#else
            QImage subImage(sub_w, sub_h, QImage::Format_RGBA8888);
            fbo->read_color(0, subImage.bits(), GL_RGBA);
#endif

            // Copy subImage in image
            for (int ii = 0; ii < sub_w; ii++) {
                int fi = i*sub_w + ii;
                if (fi == image.width())
                    break;
                for (int jj = 0; jj < sub_h; jj++) {
                    int fj = j*sub_h + jj;
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
			progress.next(false);
			// this very important (the progress bar may interfere the framebuffer
			makeCurrent();
#endif
        }
    }

    delete fbo;

    // restore the projection matrix
    camera()->set_projection_matrix(proj_matrix);

    // restore the clear color
	func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
    doneCurrent();

    bool saved = image.save(file_name);
    progress.done();

    return saved;
}


#include <QFileInfo>
#include <QFileDialog>

#include <easy3d/renderer/walk_throuth.h>
#include <easy3d/renderer/key_frame_interpolator.h>


// true to start the recording and false to stop the recording.
bool PaintCanvas::recordAnimation(bool b) {
    auto kfi = walkThrough()->interpolator();
    if (kfi->numberOfKeyFrames() == 0) {
        if (b)
            LOG(WARNING) << "recording aborted (camera path is empty). You may import a camera path from a file or"
                            " creat it by adding key frames";
        window_->stopRecordAnimation();
        return false;
    }

    // must be static because of the lambda function called in the animation thread
    static QString recordDir;
    static int snapshotCounter = 0;
    static QMetaObject::Connection connection;

    auto snapshotFramebuffer = [&]() -> void {
        const QString count = QString("%1").arg(snapshotCounter++, 4, 10,QChar('0'));
        const QString fileName = recordDir +  QString("/snapshot-") + count + QString(".png");
        raise(); // to correctly grab the frame buffer, the viewer window must be raised in front of other windows
        const QImage snapshot = grabFramebuffer();
        if (!snapshot.save(fileName, "png"))
            LOG(WARNING) << "unable to save snapshot in " << fileName.toStdString();
    };

    auto interpolationFinished = [this]() -> void { emit recordingFinished(); };

    if (b) {
        std::string model_dir = file_system::parent_directory(currentModel()->name());
        recordDir = QFileDialog::getExistingDirectory(
                this, tr("Please choose a directory"), QString::fromStdString(model_dir)
        );
        if (recordDir.isEmpty()) {
            window_->stopRecordAnimation();
            return false;
        }

        snapshotCounter = 0;
        connection = QObject::connect(this, &PaintCanvas::drawFinished, snapshotFramebuffer);

#if 0
        // Liangliang: MainWindow::stopRecordAnimation will be called from the animation thread, which causes the
        //             error: "QObject::startTimer: Timers cannot be started from another thread". To workaround,
        //             I invoke MainWindow::stopRecordAnimation by a viewer signal.
//        easy3d::connect(kfi, window_, &MainWindow::stopRecordAnimation);
#else
        easy3d::connect(&kfi->end_reached, 0, interpolationFinished);
        QObject::connect(this, &PaintCanvas::recordingFinished, window_, &MainWindow::stopRecordAnimation);
#endif

        if (kfi->interpolationIsStarted()) // stop first if is playing now
            kfi->stopInterpolation();
        playCameraPath();
        LOG(INFO) << "recording animation started...";
    }
    else {
        QObject::disconnect(connection);
        easy3d::disconnect(&kfi->end_reached, 0);
        QObject::disconnect(this, &PaintCanvas::recordingFinished, window_, &MainWindow::stopRecordAnimation);

        if (kfi->interpolationIsStarted())
            kfi->stopInterpolation();

        LOG(INFO) << "recording animation finished. " << snapshotCounter << " snapshots recorded";
    }

    return true;
}