#ifndef EASY3D_TEXT_MESH_H
#define EASY3D_TEXT_MESH_H

#include <string>

#include <easy3d/core/types.h>


namespace easy3d {

    class SurfaceMesh;

    class TextMesh {
    public:
        /**
         * @brief Create an instance of TextMesh.
         * @param font_file The full path name to a TrueType font file (normally with an "ttf" extension).
         * @param font_height The height of the font.
         * @param bezier_steps
         */
        TextMesh(const std::string &font_file = "fonts/hp.ttf", int font_height = 48, unsigned short bezier_steps = 4);

        ~TextMesh();

        /**
         * @brief Query if TextMesh was successfully initialized.
         * @return true on success and false on failure.
         */
        bool is_ready() const { return ready_; }

        /**
         * @brief Generate a 3D surface mesh of a text.
         * @param text The input text.
         * @param extrude The height (in the Z direction) of the 3D model.
         * @return The generated triangular surface mesh.
         */
        SurfaceMesh *generate_mesh(const std::string &text, float extrude = 16);

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
         * @param contours The contours of the text. The generated contours are simply appended to his variable.
         */
        void generate_contours(const std::string &text, std::vector<CharContour> &contours);

        /**
         * @brief Generate contours for a single character.
         * @param character The input character.
         * @param offset The offset of this character. In return, the new value for the subsequent character.
         * @return The contours for this character.
         */
        CharContour generate_contours(char character, float &offset);

    private:
        void *face_;
        bool ready_;

        unsigned short bezier_steps_;
        float scaling_;

        unsigned int prev_char_index_;
        unsigned int cur_char_index_;
        signed long prev_rsb_delta_;;
    };
}


#endif  // EASY3D_TEXT_MESH_H
