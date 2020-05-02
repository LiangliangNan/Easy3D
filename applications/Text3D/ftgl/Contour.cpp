/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
 * Copyright (c) 2008 Éric Beets <ericbeets@free.fr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Contour.h"

#include <math.h>

void Contour::AddPoint(Point point)
{
    if(pointList.empty() || (point != pointList[pointList.size() - 1]
                              && point != pointList[0]))
    {
        pointList.push_back(point);
    }

    if(minx > point.X())
        minx = point.X();

    if(miny > point.Y())
        miny = point.Y();

    if(maxx < point.X())
        maxx = point.X();

    if(maxy < point.Y())
        maxy = point.Y();
}


void Contour::AddOutsetPoint(Point point)
{
    outsetPointList.push_back(point);
}


void Contour::evaluateQuadraticCurve(Point A, Point B, Point C, unsigned short bezierSteps)
{
    for(unsigned int i = 1; i < bezierSteps; i++)
    {
        float t = static_cast<float>(i) / bezierSteps;

        Point U = (1.0f - t) * A + t * B;
        Point V = (1.0f - t) * B + t * C;

        AddPoint((1.0f - t) * U + t * V);
    }
}


void Contour::evaluateCubicCurve(Point A, Point B, Point C, Point D, unsigned short bezierSteps)
{
    for(unsigned int i = 0; i < bezierSteps; i++)
    {
        float t = static_cast<float>(i) / bezierSteps;

        Point U = (1.0f - t) * A + t * B;
        Point V = (1.0f - t) * B + t * C;
        Point W = (1.0f - t) * C + t * D;

        Point M = (1.0f - t) * U + t * V;
        Point N = (1.0f - t) * V + t * W;

        AddPoint((1.0f - t) * M + t * N);
    }
}


// This function is a bit tricky. Given a path ABC, it returns the
// coordinates of the outset point facing B on the left at a distance
// of 64.0.
//                                         M
//                            - - - - - - X
//                             ^         / '
//                             | 64.0   /   '
//  X---->-----X     ==>    X--v-------X     '
// A          B \          A          B \   .>'
//               \                       \<'  64.0
//                \                       \                  .
//                 \                       \                 .
//                C X                     C X
//
Point Contour::ComputeOutsetPoint(Point A, Point B, Point C)
{
    /* Build the rotation matrix from 'ba' vector */
    Point ba = (A - B).Normalise();
    Point bc = C - B;

    /* Rotate bc to the left */
    Point tmp(bc.X() * -ba.X() + bc.Y() * -ba.Y(),
                bc.X() * ba.Y() + bc.Y() * -ba.X());

    /* Compute the vector bisecting 'abc' */
    double norm = sqrt(tmp.X() * tmp.X() + tmp.Y() * tmp.Y());
    double dist = 64.0 * sqrt((norm - tmp.X()) / (norm + tmp.X()));
    tmp.X(tmp.Y() < 0.0 ? dist : -dist);
    tmp.Y(64.0);

    /* Rotate the new bc to the right */
    return Point(tmp.X() * -ba.X() + tmp.Y() * ba.Y(),
                   tmp.X() * -ba.Y() + tmp.Y() * -ba.X());
}


void Contour::SetParity(int parity)
{
    size_t size = PointCount();
    Point vOutset;

    if(((parity & 1) && clockwise) || (!(parity & 1) && !clockwise))
    {
        // Contour orientation is wrong! We must reverse all points.
        // FIXME: could it be worth writing FTVector::reverse() for this?
        for(size_t i = 0; i < size / 2; i++)
        {
            Point tmp = pointList[i];
            pointList[i] = pointList[size - 1 - i];
            pointList[size - 1 -i] = tmp;
        }

        clockwise = !clockwise;
    }

    for(size_t i = 0; i < size; i++)
    {
        size_t prev, cur, next;

        prev = (i + size - 1) % size;
        cur = i;
        next = (i + size + 1) % size;

        vOutset = ComputeOutsetPoint(GetPoint(prev), GetPoint(cur), GetPoint(next));
        AddOutsetPoint(vOutset);
    }
}


Contour::Contour(FT_Vector* contour, char* tags, unsigned int n, unsigned short bezierSteps)
{
    Point prev, cur(contour[(n - 1) % n]), next(contour[0]);
    Point a, b = next - cur;
    double olddir, dir = atan2((next - cur).Y(), (next - cur).X());
    double angle = 0.0;

    minx = 65000.0f;
    miny = 65000.0f;
    maxx = -65000.0f;
    maxy = -65000.0f;
    // See http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for(unsigned int i = 0; i < n; i++)
    {
        prev = cur;
        cur = next;
        next = Point(contour[(i + 1) % n]);
        olddir = dir;
        dir = atan2((next - cur).Y(), (next - cur).X());

        // Compute our path's new direction.
        double t = dir - olddir;
        if(t < -M_PI) t += 2 * M_PI;
        if(t > M_PI) t -= 2 * M_PI;
        angle += t;

        // Only process point tags we know.
        if(n < 2 || FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_On)
        {
            AddPoint(cur);
        }
        else if(FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Conic)
        {
            Point prev2 = prev, next2 = next;

            // Previous point is either the real previous point (an "on"
            // point), or the midpoint between the current one and the
            // previous "conic off" point.
            if(FT_CURVE_TAG(tags[(i - 1 + n) % n]) == FT_Curve_Tag_Conic)
            {
                prev2 = (cur + prev) * 0.5;
                AddPoint(prev2);
            }

            // Next point is either the real next point or the midpoint.
            if(FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Conic)
            {
                next2 = (cur + next) * 0.5;
            }

            evaluateQuadraticCurve(prev2, cur, next2, bezierSteps);
        }
        else if(FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Cubic
                 && FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Cubic)
        {
            evaluateCubicCurve(prev, cur, next, Point(contour[(i + 2) % n]), bezierSteps);
        }
    }

    // If final angle is positive (+2PI), it's an anti-clockwise contour,
    // otherwise (-2PI) it's clockwise.
    clockwise = (angle < 0.0);
}
