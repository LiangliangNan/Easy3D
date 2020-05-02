#ifndef EASY3D_TEXT3D_H
#define EASY3D_TEXT3D_H

#include <string>

#include <easy3d/core/types.h>

namespace easy3d {

    class SurfaceMesh;

    class Text3D {
    public:
        /**
         * @brief Create an instance of Text3D
         * @param font_file The full path name to a TrueType font file (normally with an "ttf" extension).
         * @param font_height The height of the font.
         */
        Text3D(const std::string &font_file = "fonts/hp.ttf", int font_height = 48);
        ~Text3D();

        /**
         * @brief Query if Text3D was successfully initialized.
         * @return true on success and false on failure.
         */
        bool is_ready() const { return ready_; }

        /**
         * @brief Generate a 3D surface mesh of a text.
         * @param text The input text.
         * @param extrude
         * @param bezierSteps
         * @return
         */
        SurfaceMesh *generate_mesh(const std::string& text, float extrude = 16, unsigned short bezierSteps = 4) const;

        /**
         * @brief Generate contours for a text.
         * @param text The input text.
         * @param contours The contours in the XOY-plane (each contour is a closed polygon). The generated contours are
         *                 simply appended to the end of this variable.
         * @param bezierSteps
         * @return The contours in the XOY-plane. Each contour is a closed polyline.
         */
        void generate_contours(const std::string& text, std::vector<Polygon2>& contours, unsigned short bezierSteps = 4) const;

        /**
         * @brief Generate contours for a single character.
         * @param character The input character.
         * @param offset The offset of this character.
         * @param contours The contours in the XOY-plane (each contour is a closed polygon). The generated contours are
         *                 simply appended to the end of this variable.
         * @param bezierSteps
         * @return The offset value for the subsequent character in the text.
         */
        float generate_contours(char character, float offset, std::vector<Polygon2>& contours, unsigned short bezierSteps = 4) const;

    private:
        void* face_;
        bool  ready_;
    };
}


#endif  // EASY3D_TEXT3D_H
