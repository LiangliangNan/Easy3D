/*
 *  SimpleARQGLViewer.cpp
 *  Creating an AR application with QGLViewer
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

#include <opencv2/opencv.hpp>

#include <iostream>
#include <algorithm>

#include "OGL_OCV_common.h"
#include <GLUT/glut.h>

#include <qapplication.h>
#include <QBasicTimer>
#include <QGLViewer/qglviewer.h>

using namespace cv;
using namespace std;

class Tracker {
private:
    //...
    Mat raux,taux; //keep R and t for next frame
    //...
public:
    //...

    void process(const Mat& frame) {
        // track 2D features and match them to known 3D points
    }

    //...

    void calcModelViewMatrix(Mat& modelview_matrix, const Mat& camMat) {
        vector<Point2f> ObjPoints,ImagePoints;
        // Setup ObjPoints and ImagePoints so they correspond 3D -> 2D

        cv::Mat Rvec,Tvec;
        cv::solvePnP(ObjPoints, ImagePoints, camMat, Mat(), raux, taux, !raux.empty());
        raux.convertTo(Rvec,CV_32F);
        taux.convertTo(Tvec ,CV_64F);

        Mat Rot(3,3,CV_32FC1);
        Rodrigues(Rvec, Rot);

        // [R | t] matrix
        Mat_<double> para = Mat_<double>::eye(4,4);
        Rot.convertTo(para(Rect(0,0,3,3)),CV_64F);
        Tvec.copyTo(para(Rect(3,0,1,3)));

        Mat cvToGl = Mat::zeros(4, 4, CV_64F);
        cvToGl.at<double>(0, 0) = 1.0f;
        cvToGl.at<double>(1, 1) = -1.0f; // Invert the y axis
        cvToGl.at<double>(2, 2) = -1.0f; // invert the z axis
        cvToGl.at<double>(3, 3) = 1.0f;

        para = cvToGl * para;

        Mat(para.t()).copyTo(modelview_matrix); // transpose to col-major for OpenGL
    }
};

class OpenCVCamera : public qglviewer::Camera {
public:
    Mat camMat;

    virtual void loadProjectionMatrix(bool reset) const {
        static Mat_<double> persp;
        double near = 1, far = 100.0;

        glMatrixMode(GL_PROJECTION);
        if(persp.empty()) {
            persp.create(4,4); persp.setTo(0);

            // http://kgeorge.github.io/2014/03/08/calculating-opengl-perspective-matrix-from-opencv-intrinsic-matrix/
            double fx = camMat.at<float>(0,0);
            double fy = camMat.at<float>(1,1);
            double cx = camMat.at<float>(0,2);
            double cy = camMat.at<float>(1,2);
            persp(0,0) = fx/cx;
            persp(1,1) = fy/cy;
            persp(2,2) = -(far+near)/(far-near);
            persp(2,3) = -2.0*far*near / (far-near);
            persp(3,2) = -1.0;

            cout << "perspective m \n" << persp << endl;

            persp = persp.t(); //to col-major
        }
        glLoadMatrixd((double*)persp.data);
    }
};

