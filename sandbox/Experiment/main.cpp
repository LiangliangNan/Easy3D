#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/core/hash.h>

using namespace easy3d;


#define STB_TRUETYPE_IMPLEMENTATION

#include <3rd_party/stb/stb_truetype.h>


#include <xmmintrin.h>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <numeric>
#include <vector>
#include <fstream>


int readFile(char const *filename, unsigned char **out) {
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    *out = (unsigned char *) malloc(size + 1);
    fread(*out, size, 1, fp);
    (*out)[size] = 0;
    fclose(fp);
    return size;
}

int main(int argc, char **argv) {
    std::ofstream output("easy3d.xyz");

    const std::string font_file = resource::directory() + "/fonts/en_Earth-Normal.ttf";
    const std::string text = "Easy3D";

    // load font
    unsigned char *ttf;
    readFile(font_file.c_str(), &ttf);
    stbtt_fontinfo font;
    stbtt_InitFont(&font, ttf, stbtt_GetFontOffsetForIndex(ttf, 0));

    float size_in_pixels = 20.0f;
    float scale = stbtt_ScaleForPixelHeight(&font, size_in_pixels);
    int ascent,descent,linegap;
    stbtt_GetFontVMetrics(&font,&ascent,&descent,&linegap);


    int offset = 0;
    // load text
    for (int i = 0; i < text.length(); i++) {
        int glyph = stbtt_FindGlyphIndex(&font, text[i]);
        stbtt_vertex *vertices;
        int num_verts = stbtt_GetGlyphShape(&font, glyph, &vertices);

//        std::cout << "\n\n" << text[i] << ": " << num_verts << std::endl;
        for (int j=0; j<num_verts; ++j) {
            output << vertices[j].x << " " << vertices[j].y + offset << " " << 0 << std::endl;
        }

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, text[i], &advanceWidth, &leftSideBearing);
        offset += advanceWidth - leftSideBearing;

        stbtt_FreeShape(&font, vertices);
    }

    return 0;
}
