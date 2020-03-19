//
//  ofxFontStash.h
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

#ifndef ofxFontStash_h
#define ofxFontStash_h


#include <vector>

struct sth_stash;

namespace easy3d {

    class ofxFontStash {

    public:

        /**
         * Constructor.
         * @param texture_size  The size of the font textures, e.g., 512, 1024, 2048.
         * @param mipmaps True to create mipmaps for the font textures.
         */
        ofxFontStash(int texture_size = 512, bool mipmaps = false);
        ~ofxFontStash();

        /**
         * Add a font.
         * @param font_file The font file (.ttf, etc).
         */
        bool add_font(const std::string &font_file);

        /**
         * The number of available fonts.
         */
        int num_fonts() const { return font_ids_.size(); }

        //will draw text in one line, ignoring "\n"'s
        //fontID - draw font with fontID. FontID generated with each added font. 0 is for first/only font, 1 is for second, 2 is third and so on
        float draw(const std::string &text, float size, float x, float y, int fontID = 0);

        float getFontHeight(float fontSize);

    private:
        struct sth_stash *stash;
        int texture_size_;
        std::vector<int> font_ids_;
    };

}

#endif
