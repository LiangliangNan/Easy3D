#ifndef EASY3D_TEXT3D_H
#define EASY3D_TEXT3D_H

#include <string>


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
        SurfaceMesh *generate(const std::string text = "HelloWorld", float extrude = 16, unsigned short bezierSteps = 4);

    private:
        void* face_;
        bool  ready_;
    };
}


#endif  // EASY3D_TEXT3D_H
