/*
 *  OGL_OCV_common.h
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
#pragma once

#include <windows.h>
#include <opencv2/opencv.hpp>

using namespace cv;

#if defined(__APPLE__)
#  include <OpenGL/gl.h>
#elif defined(__linux__) || defined(__MINGW32__) || defined(WIN32)
#  define GLEW_STATIC
//#  include <GL/glew.h>
//#  include <GL/wglew.h>
#  include <GL/gl.h>
#  include <GL/glu.h>
//#  include <GL/glext.h>
#else
#  include <gl.h>
#endif

namespace RS {

	Mat copyImgToTex(const Mat& _tex_img, GLuint* texID, double* _twr, double* _thr);

	typedef struct my_texture {
		GLuint tex_id;
		double twr, thr, aspect_w2h;
		Mat image, tex_pow2;
		my_texture() :tex_id(-1), twr(1.0), thr(1.0) {}
		bool initialized;
		void set(const Mat& ocvimg) {
			if (tex_pow2.empty()) {
				ocvimg.copyTo(image);
				copyImgToTex(image, &tex_id, &twr, &thr);
				aspect_w2h = (double)ocvimg.cols / (double)ocvimg.rows;
			}
			else {
				Mat region = tex_pow2(Rect(0, 0, ocvimg.cols, ocvimg.rows));
				if (ocvimg.type() == region.type()) {
					flip(ocvimg, region, 0);
				}
				else {
					if (ocvimg.type() == CV_8UC1) {
						cvtColor(ocvimg, region, CV_GRAY2BGR);
					}
					else {
						ocvimg.convertTo(region, CV_8UC3, 255.0);
					}
					flip(region, region, 0);
				}

				glBindTexture(GL_TEXTURE_2D, tex_id);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, tex_pow2.cols, tex_pow2.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, tex_pow2.data);
			}
		}
	} OpenCVGLTexture;

	void glEnable2D();							// setup 2D drawing
	void glEnable2D(int w, int h, int x, int y);	// setup 2D drawing with viewport & postion setting
	void glDisable2D(); 						// end 2D drawing
	void glDisable2Dvp();						// end 2D drawing and pop viewport attrib
	OpenCVGLTexture MakeOpenCVGLTexture(const Mat& _tex_img); // create an OpenCV-OpenGL image

	// draw an OpenCV-OpenGL image
	void drawOpenCVImageInGLOnlyQuad(const OpenCVGLTexture& tex, int width, int height);
	void drawOpenCVImageInGLFullViewport(const OpenCVGLTexture& tex);

}