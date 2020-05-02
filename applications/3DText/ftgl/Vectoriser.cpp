/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Éric Beets <ericbeets@free.fr>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
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

#include "Vectoriser.h"

Vectoriser::Vectoriser(const FT_GlyphSlot glyph, unsigned short bezierSteps)
:   contourList(0),
    ftContourCount(0),
    contourFlag(0)
{
    if(glyph)
    {
        outline = glyph->outline;

        ftContourCount = outline.n_contours;
        contourList = 0;
        contourFlag = outline.flags;

        ProcessContours(bezierSteps);
    }
}


Vectoriser::~Vectoriser()
{
    for(size_t c = 0; c < ContourCount(); ++c)
    {
        delete contourList[c];
    }

    delete [] contourList;
}


void Vectoriser::ProcessContours(unsigned short bezierSteps)
{
    short contourLength = 0;
    short startIndex = 0;
    short endIndex = 0;

    contourList = new Contour*[ftContourCount];

    for(int i = 0; i < ftContourCount; ++i)
    {
        FT_Vector* pointList = &outline.points[startIndex];
        char* tagList = &outline.tags[startIndex];

        endIndex = outline.contours[i];
        contourLength =  (endIndex - startIndex) + 1;

        Contour* contour = new Contour(pointList, tagList, contourLength, bezierSteps);

        contourList[i] = contour;

        startIndex = endIndex + 1;
    }

    // Compute each contour's parity. FIXME: see if FT_Outline_Get_Orientation
    // can do it for us.
    for(int i = 0; i < ftContourCount; i++)
    {
        Contour *c1 = contourList[i];

        // 1. Find the leftmost point.
        Point leftmost(65536.0, 0.0);

        for(size_t n = 0; n < c1->PointCount(); n++)
        {
            Point p = c1->GetPoint(n);
            if(p.X() < leftmost.X())
            {
                leftmost = p;
            }
        }

        // 2. Count how many other contours we cross when going further to
        // the left.
        int parity = 0;

        for(int j = 0; j < ftContourCount; j++)
        {
            if(j == i)
            {
                continue;
            }

            Contour *c2 = contourList[j];

            for(size_t n = 0; n < c2->PointCount(); n++)
            {
                Point p1 = c2->GetPoint(n);
                Point p2 = c2->GetPoint((n + 1) % c2->PointCount());

                /* FIXME: combinations of >= > <= and < do not seem stable */
                if((p1.Y() < leftmost.Y() && p2.Y() < leftmost.Y())
                    || (p1.Y() >= leftmost.Y() && p2.Y() >= leftmost.Y())
                    || (p1.X() > leftmost.X() && p2.X() > leftmost.X()))
                {
                    continue;
                }
                else if(p1.X() < leftmost.X() && p2.X() < leftmost.X())
                {
                    parity++;
                }
                else
                {
                    Point a = p1 - leftmost;
                    Point b = p2 - leftmost;
                    if(b.X() * a.Y() > b.Y() * a.X())
                    {
                        parity++;
                    }
                }
            }
        }

        // 3. Make sure the glyph has the proper parity.
        c1->SetParity(parity);
    }
}


size_t Vectoriser::PointCount()
{
    size_t s = 0;
    for(size_t c = 0; c < ContourCount(); ++c)
    {
        s += contourList[c]->PointCount();
    }

    return s;
}


const Contour* const Vectoriser::GetContour(size_t index) const
{
    return (index < ContourCount()) ? contourList[index] : NULL;
}


