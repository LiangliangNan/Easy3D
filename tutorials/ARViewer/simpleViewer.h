#ifndef AR_VIEWER_H
#define AR_VIEWER_H

#include <windows.h>
#include <QGLViewer/qglviewer.h>
#include "opencv_camera.h"
#include "OGL_OCV_common.h"

class Viewer : public QGLViewer
{
protected:
	VideoCapture            vc;
	Mat                     frame;
	Mat                     orig_gray;
	RS::OpenCVGLTexture     ocv_tex;
	Tracker                 tracker;
	Mat_<float>             camMat;
	Mat_<double>            modelViewMatrix;
	QBasicTimer*            frameTimer;
	QBasicTimer*            detectorTimer;

public:
	Viewer() {

		vc.open("myvideo.MOV");
		if (!vc.isOpened()) {
			cerr << "can't open video\n";
		}
		else {
			Mat frame_, orig, orig_warped, tmp;
			vc >> frame_;
			if (frame_.empty()) {
				cerr << "can't get first frame\n";
			}
			else {
				frame_.copyTo(frame);
				float f = std::max(frame.cols, frame.rows);
				camMat = (Mat_<float>(3, 3) << f, 0, frame.cols / 2,
					0, f, frame.rows / 2,
					0, 0, 1);

				frameTimer = new QBasicTimer();
				detectorTimer = new QBasicTimer();
			}
		}
	}
	~Viewer() {
		frameTimer->stop();
		delete frameTimer;

		detectorTimer->stop();
		delete detectorTimer;
	}

	virtual void draw() {
		drawBackground();

		glLoadMatrixd((double*)modelViewMatrix.data);

		glPushMatrix();
		glDisable(GL_LIGHTING);
		glColor3f(1, 0, 0);
		glTranslatef(0, 0, -0.3);
		glutWireCube(0.6);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	virtual void init() {
		// Enable GL textures
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Nice texture coordinate interpolation
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		ocv_tex = RS::MakeOpenCVGLTexture(frame);

		setFixedHeight(frame.rows);
		setFixedWidth(frame.cols);

		clearMouseBindings();

		frameTimer->start(1, this);
		detectorTimer->start(1, this);

		OpenCVCamera* c = new OpenCVCamera;
		c->camMat = camMat;
		setCamera(c);
	}

	void timerEvent(QTimerEvent *timer) {
		Mat frame_;
		vc >> frame_;
		if (frame_.empty()) return;

		frame_.copyTo(frame);

		tracker.process(frame);
		tracker.calcModelViewMatrix(modelViewMatrix, camMat);

		ocv_tex.set(frame);

		this->update();
	}

	void drawBackground()
	{

		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1);

		startScreenCoordinatesSystem(true);

		// Draws the background quad
		RS::drawOpenCVImageInGLOnlyQuad(ocv_tex, width(), height());

		stopScreenCoordinatesSystem();

		// Depth clear is not absolutely needed. An other option would have been to draw the
		// QUAD with a 0.999 z value (z ranges in [0, 1[ with startScreenCoordinatesSystem()).
		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}

};

#endif
