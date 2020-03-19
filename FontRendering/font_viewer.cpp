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

#include "font_viewer.h"
#include "fontstash.h"

#include <easy3d/util/dialogs.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/viewer/opengl_error.h>


using namespace easy3d;


/* create a font stash with a maximum texture size of 512 x 512 */
struct sth_stash *stash = nullptr;
/* load truetype font */
int droid = -1;

FontViewer::FontViewer(const std::string &title)
        : Viewer(title) {
    set_background_color(vec3(1, 1, 1));
}


void FontViewer::init() {
    Viewer::init();

    LOG(WARNING) << "TODO: add a OpenGLText class, allowing changing text color/font/size etc.";

    /* create a font stash with a maximum texture size of 512 x 512 */
    stash = sth_create(512, 512); easy3d_debug_log_gl_error;

    /* load truetype font */
    //const std::string font_file = resource::directory() + "/fonts/DroidSerif-Regular.ttf";
    const std::string font_file = resource::directory() + "/fonts/zachary.ttf";
    droid = sth_add_font(stash, font_file.c_str()); easy3d_debug_log_gl_error;
}


void FontViewer::cleanup() {
    Viewer::cleanup();

    sth_delete(stash);
}


void FontViewer::draw() const {
    Viewer::draw();


    /* position of the text */
    float font_size = 128.0f;
    float x = 50;
    float y = 50;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* draw text during your OpenGL render loop */
    sth_begin_draw(stash); easy3d_debug_log_gl_error;
    /* position: (x, y); font size: 24 */

    sth_draw_text(stash, droid, font_size, x, y, "Hello world! ", &x);easy3d_debug_log_gl_error;
    sth_draw_text(stash, droid, font_size, x, y, "I Love Easy3D!", &x); easy3d_debug_log_gl_error;

    /* now, the float x contains the x position of the next char */
    sth_end_draw(stash); easy3d_debug_log_gl_error;

    glDisable(GL_BLEND);
}
