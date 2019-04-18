
/*
 *  OGL_OCV_common.cpp
 *  Common interop between OpenCV and OpenGL
 *
 *  Created by Roy Shilkrot on 2/16/2015
 *  Copyright 2015 Roy Shilkrot. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *
 */

#include "OGL_OCV_common.h"

namespace RS {

	void drawOpenCVImageInGLOnlyQuad(const OpenCVGLTexture& tex, int width, int height) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);

		double ithr = tex.thr, itwr = tex.twr;
		double n[3] = { 0,0,-1 };

		glBegin(GL_QUADS);

		glNormal3dv(n);
		glTexCoord2d(0, 0);
		glVertex2d(0, 0);

		glTexCoord2d(0, ithr);
		glVertex2d(0, height);

		glTexCoord2d(itwr, ithr);
		glVertex2d(width, height);

		glTexCoord2d(itwr, 0);
		glVertex2d(width, 0);

		glEnd();
	}

	void drawOpenCVImageInGLFullViewport(const OpenCVGLTexture& tex) {
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);

		int vPort[4]; glGetIntegerv(GL_VIEWPORT, vPort);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glPushMatrix();
		glColor3ub(255, 255, 255);

		glScaled(vPort[3], vPort[3], 1);

		double aw2h = tex.aspect_w2h, ithr = tex.thr, itwr = tex.twr;
		double n[3] = { 0,0,-1 };

		GLint face_ori; glGetIntegerv(GL_FRONT_FACE, &face_ori);
		glFrontFace(GL_CW); //we're gonna draw clockwise

		glBegin(GL_QUADS);

		glNormal3dv(n);
		glTexCoord2d(0, 0);
		glVertex2d(0, 0);

		glTexCoord2d(0, ithr);
		glVertex2d(0, 1);

		glTexCoord2d(itwr, ithr);
		glVertex2d(aw2h, 1);

		glTexCoord2d(itwr, 0);
		glVertex2d(aw2h, 0);

		glEnd();
		glPopMatrix();

		glFrontFace(face_ori); //restore front face orientation

		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
	}

	void glEnable2D()
	{
		glPushAttrib(GL_ENABLE_BIT);
		glPushAttrib(GL_CURRENT_BIT);

		int vPort[4];
		glGetIntegerv(GL_VIEWPORT, vPort);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, vPort[2], 0, vPort[3], -1, 4);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslated(0.375, 0.375, 0);

		//	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // clear the screen

		//	glDisable(GL_DEPTH_TEST);	
	}

	void glEnable2D(int w, int h, int x, int y) {
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(x, y, w, h);

		RS::glEnable2D();

		glScaled(w, h, 1);
	}

	void glDisable2D()
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		//	glEnable(GL_DEPTH_TEST);

		glPopAttrib();
		glPopAttrib();

	}

	void glDisable2Dvp() {
		glDisable2D();
		glPopAttrib();
	}


#if defined(WIN32)
#define log2(x) log10((double)(x))/log10(2.0)
#endif

	Mat copyImgToTex(const Mat& _tex_img, GLuint* texID, double* _twr, double* _thr) {
		Mat tex_img = _tex_img;
		flip(_tex_img, tex_img, 0);
		Mat tex_pow2(pow(2.0, ceil(log2(tex_img.rows))), pow(2.0, ceil(log2(tex_img.cols))), CV_8UC3);
		//	std::cout << "original image size " << tex_img.size() << std::endl;
		//	std::cout << "adjusted to OpenGL texture: " << tex_pow2.size() << std::endl;
		Mat region = tex_pow2(Rect(0, 0, tex_img.cols, tex_img.rows));
		if (tex_img.type() == region.type()) {
			tex_img.copyTo(region);
		}
		else if (tex_img.type() == CV_8UC1) {
			cvtColor(tex_img, region, CV_GRAY2BGR);
		}
		else {
			tex_img.convertTo(region, CV_8UC3, 255.0);
		}

		if (_twr != 0 && _thr != 0) {
			*_twr = (double)tex_img.cols / (double)tex_pow2.cols;
			*_thr = (double)tex_img.rows / (double)tex_pow2.rows;
		}
		glBindTexture(GL_TEXTURE_2D, *texID);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, tex_pow2.cols, tex_pow2.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, tex_pow2.data);
		return tex_pow2;
	}

	OpenCVGLTexture MakeOpenCVGLTexture(const Mat& _tex_img) {
		OpenCVGLTexture _ocvgl;

		glGenTextures(1, &_ocvgl.tex_id);
		glBindTexture(GL_TEXTURE_2D, _ocvgl.tex_id);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (!_tex_img.empty()) { //image may be dummy, just to generate pointer to texture object in GL
			copyImgToTex(_tex_img, &_ocvgl.tex_id, &_ocvgl.twr, &_ocvgl.thr);
			_ocvgl.aspect_w2h = (double)_tex_img.cols / (double)_tex_img.rows;
		}

		return _ocvgl;
	}

}