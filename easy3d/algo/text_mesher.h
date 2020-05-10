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


#ifndef EASY3D_ALGO_TEXT_MESHER_H
#define EASY3D_ALGO_TEXT_MESHER_H

#include <string>

#include <easy3d/core/types.h>


namespace easy3d {

    class SurfaceMesh;

    class TextMesher {
    public:
        /**
         * @brief Create an instance of TextMesher.
         * @param font_file The full path name to a TrueType font file (normally with an "ttf" extension).
         * @param font_size The size of the font.
         * @note The font will be used in all subsequent generation procedure until the next call to set_font().
         */
        TextMesher(const std::string &font_file, int font_size);

        ~TextMesher();

        /**
         * @brief Change the font.
         * @param font_file The full path name to a TrueType font file (normally with an "ttf" extension).
         * @param font_size The size of the font.
         * @note The font will be used in all subsequent generation procedure until the next call to set_font().
         */
        void set_font(const std::string &font_file, int font_size);

        /**
         * @brief Generate a 3D surface mesh of a text.
         * @param text The input text.
         * @param x The x-coordinate of the starting position.
         * @param y The y-coordinate of the starting position.
         * @param extrude The height (in the Z direction) of the 3D model.
         * @param collision_free If true, the final mesh will be free of intersections between characters.
         * @return The generated triangular surface mesh.
         */
        SurfaceMesh *generate(const std::string &text, float x, float y, float extrude, bool collision_free = false);

        /**
         * @brief Generate 3D surface representation of a text and append the surface to an existing mesh.
         * @param text The input text.
         * @param x The x-coordinate of the starting position.
         * @param y The y-coordinate of the starting position.
         * @param extrude The height (in the Z direction) of the 3D model.
         * @param collision_free If true, the final mesh will be free of intersections between characters.
         * @param True on success and false on failure.
         */
        bool generate(SurfaceMesh* mesh, const std::string &text, float x, float y, float extrude, bool collision_free = false);

        /**
         * @brief Generate contours from a text.
         * @param text The input text.
         * @param x The x-coordinate of the starting position.
         * @param y The y-coordinate of the starting position.
         * @param contours The contours of the text (each character may have multiple contours). The generated contours
         *        are simply appended to his variable.
         * @param collision_free If true, the generated contours will be free of intersections between characters.
         */
        bool generate(const std::string &text, float x, float y, std::vector< std::vector<Polygon2> > &contours, bool collision_free);

    private:
        void cleanup();

        /**
         * @brief Generate 3D surface representation of a text and append the surface to an existing mesh.
         * @param text The input text.
         * @param x The x-coordinate of the starting position.
         * @param y The y-coordinate of the starting position.
         * @param extrude The height (in the Z direction) of the 3D model.
         * @param collision_free True the mesh will be the union of all the characters.
         * @param True on success and false on failure.
         */
        bool _generate(SurfaceMesh* mesh, const std::wstring &text, float x, float y, float extrude, bool collision_free);

        /**
         * @brief Generate contours for a text.
         * @param text The input text.
         * @param x The x-coordinate of the starting position.
         * @param y The y-coordinate of the starting position.
         * @param contours The contours of the text. The generated contours are simply appended to his variable.
         * @param collision_free True the contours will be the union of the contours of all the characters.
         */
        bool _generate_contours(const std::wstring &text, float x, float y, std::vector< std::vector<Polygon2> > &contours, bool collision_free);

        /**
         * @brief Generate contours for a single character.
         * @param c The input character.
         * @param x The x-coordinate of the starting position. In return, the new value for the subsequent character.
         * @param y The y-coordinate of the starting position. In return, the new value for the subsequent character.
         * @param contours The contours of this character.
         * @note All generated contours are in CCW orientation, but they may have intersections.
         */
        bool _generate_contours(wchar_t c, float& x, float& y, std::vector<Polygon2>& contours);

    private:
        void *font_library_;
        void *font_face_;
        bool ready_;

        std::string font_file_;
        int font_size_;

        // Controls the smoothness of the curved corners. A greater value results in a smooth transitions but more
        // vertices. Suggested value is 4.
        unsigned short bezier_steps_;

        unsigned int prev_char_index_;
        signed long prev_rsb_delta_;;
    };
}


#endif  // EASY3D_ALGO_TEXT_MESHER_H
