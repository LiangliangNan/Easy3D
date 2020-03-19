//
//  ofxFontStash.cpp
//  fontsExample
//
//  Created by Oriol Ferrer Mesià on 22/05/12.
//  Copyright (c) 2012 uri.cat. All rights reserved.
//
/*
 
 The MIT License
 
 Copyright (c) 2012, Oriol Ferrer Mesià.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */


#include <iostream>
#include "opengl_text.h"
#include "fontstash.h"

#include <easy3d/fileio/resources.h>

namespace easy3d {

    ofxFontStash::ofxFontStash(int texture_size, bool mipmaps) {
        texture_size_ = geom::next_pow2(texture_size);
        stash = sth_create(texture_size_, texture_size_, mipmaps);
    }

    ofxFontStash::~ofxFontStash() {
        if (stash != nullptr)
            sth_delete(stash);
    }

    bool ofxFontStash::add_font(const std::string &font_file) {
        if (stash == NULL) {
            LOG(ERROR) << "addFont() error: font stash not initialized, call setup first";
            return false;
        }

        int fontId = sth_add_font(stash, font_file.c_str());
        if (fontId <= 0) {
            LOG(ERROR) << "could not load font: "<< font_file;
            return false;
        }

        font_ids_.push_back(fontId);

        LOG(INFO) << "Loaded font '" << font_file << "' in texture (" << texture_size_ << " x " << texture_size_ << ")";
        return true;
    }


    float ofxFontStash::draw(const std::string &text, float size, float x, float y, int fontID) {
        float dx = 0.0f;
        if (stash != NULL) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            sth_begin_draw(stash);
            sth_draw_text(stash, font_ids_[fontID], size, x, y, text.c_str(), &dx); //this might draw
            sth_end_draw(stash); // this actually draws
            glDisable(GL_BLEND);
        } else {
            LOG(ERROR) << "Can't draw() without having been setup first!";
        }
        return dx;
    }


    float ofxFontStash::getFontHeight(float fontSize) {
        float asc, desc, lineh;
        sth_vmetrics(stash, font_ids_[0], fontSize, &asc, &desc, &lineh);
        return asc - desc;
    }

}