//#define USE_QT_FBO
#define SHOW_PROGRESS

#ifdef  USE_QT_FBO
#include <QOpenGLFramebufferObject>
#else
#include <easy3d/viewer/framebuffer_object.h>
#endif

#include <QOpenGLFunctions>
#include <QMessageBox>

#ifdef SHOW_PROGRESS // enable progress and cursor
#include <QProgressDialog>
#include <QGuiApplication>
#include <QCursor>
#endif

#include "paint_canvas.h"
#include "main_window.h"

#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/transform.h>
#include <easy3d/util/logging.h>


using namespace easy3d;

bool PaintCanvas::saveSnapshot(int w, int h, int samples, const QString &file_name, bool bk_white, bool expand)
{
    makeCurrent();

    int max_samples = 0;
    func_->glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    if (samples > max_samples) {
        LOG(WARNING) << "requested samples (" << samples << ") exceeds the supported maximum samples (" << max_samples << ")";
        return false;
    }

    QSize finalSize(w, h);
    int sub_w = static_cast<int>(width() * dpi_scaling());
    int sub_h = static_cast<int>(height() * dpi_scaling());
    QSize subSize(sub_w, sub_h);

    double aspectRatio = sub_w / static_cast<double>(sub_h);
    double newAspectRatio = w / static_cast<double>(h);
    double zNear = camera()->zNear();
    double zFar = camera()->zFar();

    float xMin, yMin;
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
        QMessageBox::warning(this, "Image saving error", "Unable to create resulting image", QMessageBox::Ok, QMessageBox::NoButton);
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

    // remember the current projection matrix
    // const mat4& proj_matrix = camera()->projectionMatrix(); // Liangliang: This will definitely NOT work !!!
    const mat4 proj_matrix = camera()->projectionMatrix();

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

#ifdef SHOW_PROGRESS
    QProgressDialog progress("Snapshot...", "Cancel", 0, nbX*nbY, nullptr, Qt::SplashScreen);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(false);
    progress.setAutoReset(false);
    progress.setCancelButton(nullptr);
    progress.show();
    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents();
    // progress logger changes the context thus interferes with the screen grabbing mechanism
    makeCurrent();
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
			// restore the projection matrix and color
			camera()->set_projection_matrix(proj_matrix);
			if (bk_white)
				func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

            progress.setValue(count);
            // progress logger changes the context thus interferes with the screen grabbing mechanism
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

#ifdef SHOW_PROGRESS
    progress.setLabelText("Saving image...");
    qApp->processEvents();
#endif

    bool saved = image.save(file_name);

#ifdef SHOW_PROGRESS
    progress.hide();
    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
#endif

    return saved;
}
