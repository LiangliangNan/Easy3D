#ifndef EASY3D_TEXT_MESHER_H
#define EASY3D_TEXT_MESHER_H

#include <string>

#include <easy3d/core/types.h>


namespace easy3d {

    class SurfaceMesh;

    class TextMesher {
    public:
        /**
         * @brief Create an instance of TextMesher.
         * @param font_file The full path name to a TrueType font file (normally with an "ttf" extension).
         * @param font_height The height of the font.
         * @param bezier_steps Control the smoothness of the curved corners. A greater value results in a smooth
         *                     transitions but more vertices. Suggested value is 4.
         */
        TextMesher(const std::string &font_file, int font_height = 48, unsigned short bezier_steps = 4);

        ~TextMesher();

        /**
         * @brief Generate a 3D surface mesh of a text.
         * @param text The input text.
         * @param x The x-position of the character. In return, the new value for the subsequent character.
         * @param y The y-position of the character. In return, the new value for the subsequent character.
         * @param extrude The height (in the Z direction) of the 3D model.
         * @return The generated triangular surface mesh.
         */
        SurfaceMesh *generate_mesh(const std::string &text, float x, float y, float extrude = 16);

    private:
        /**
         * A contour is a closed polygon and it has an orientation (clockwise or counter-clockwise)
         */
        struct Contour : Polygon2 {
            Contour() : clockwise(false) {}
            Contour(std::size_t size) : Polygon2(size), clockwise(false) {}
            bool clockwise;
        };

        /**
         * CharContour represents the contours of a character, which may contain multiple contours.
         */
        struct CharContour : std::vector<Contour> {
            char character;
        };

        /**
         * @brief Generate contours for a text.
         * @param text The input text.
         * @param x The x-position of the character. In return, the new value for the subsequent character.
         * @param y The y-position of the character. In return, the new value for the subsequent character.
         * @param contours The contours of the text. The generated contours are simply appended to his variable.
         */
        void generate_contours(const std::string &text, float x, float y, std::vector<CharContour> &contours);

        /**
         * @brief Generate contours for a single character.
         * @param character The input character.
         * @param x The x-position of the character. In return, the new value for the subsequent character.
         * @param y The y-position of the character. In return, the new value for the subsequent character.
         * @return The contours for this character.
         */
        CharContour generate_contours(char character, float& x, float& y);

    private:
        void *face_;
        bool ready_;

        unsigned short bezier_steps_;

        unsigned int prev_char_index_;
        unsigned int cur_char_index_;
        signed long prev_rsb_delta_;;
    };
}


#endif  // EASY3D_TEXT_MESHER_H
