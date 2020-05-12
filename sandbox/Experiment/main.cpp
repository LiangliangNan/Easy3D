#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/core/curve.h>

#include <cstring>
#include <cstdio>
#include <vector>
#include <fstream>

#define STB_TRUETYPE_IMPLEMENTATION
#include <3rd_party/stb/stb_truetype.h>


using namespace easy3d;


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


bool extract_contours(stbtt_fontinfo *font, int codePoint, std::vector<Polygon2> &contours, unsigned short bezier_steps, float offset_x, double offset_y) {
    int glyphIndex = stbtt_FindGlyphIndex(font, codePoint);
    if (glyphIndex == 0) {
        LOG(WARNING) << "undefined character code for character " << char(codePoint)
                     << " (your font may not support this character)";
        return false;
    }

    std::size_t old_num = contours.size();

    stbtt_vertex *vertices;
    const int num_verts = stbtt_GetGlyphShape(font, glyphIndex, &vertices);

    int polygonBeginIndex = 0;
    while (polygonBeginIndex < num_verts) {
        stbtt_vertex *nextPolygonBegin = std::find_if(vertices + polygonBeginIndex + 1, vertices + num_verts,
                                                      [](const stbtt_vertex &p) { return p.type == STBTT_vmove; });
        const int nextPolygonFirstIndex = std::distance(vertices, nextPolygonBegin);
        const int currentPolygonLastIndex = nextPolygonFirstIndex - 1;

        Polygon2 contour;
        for (int vi = polygonBeginIndex; vi < currentPolygonLastIndex; ++vi) {
            stbtt_vertex *v1 = vertices + vi;
            stbtt_vertex *v2 = vertices + vi + 1;

            const vec2 p1(v1->x + offset_x, v1->y + offset_y);
            const vec2 p2(v2->x + offset_x, v2->y + offset_y);
            const vec2 pc(v2->cx + offset_x, v2->cy + offset_y);
            const vec2 pc1(v2->cx1 + offset_x, v2->cy1 + offset_y);

            if (v2->type == STBTT_vline) // line
                contour.push_back(p1);
            else if (v2->type == STBTT_vcurve)  //quadratic Bezier
                curve::quadratic(p1, pc, p2, bezier_steps, contour);
            else if (v2->type == STBTT_vcubic)  //cubic Bezier
                curve::cubic(p1, pc, pc1, p2, bezier_steps, contour);
            else
                LOG(ERROR) << "unrecognized contour point type";
        }

        contours.push_back(contour);

        polygonBeginIndex = nextPolygonFirstIndex;
    }

    stbtt_FreeShape(font, vertices);

    return contours.size() > old_num;
}


int main(int argc, char **argv) {
    std::ofstream output("easy3d.xyz");

    const std::string font_file = resource::directory() + "/fonts/en_Earth-Normal.ttf";
    const std::string text = "Easy3D";

    // load font
    unsigned char *ttf;
    readFile(font_file.c_str(), &ttf);
    int font_offset = stbtt_GetFontOffsetForIndex(ttf, 0);
    if (font_offset != 0) {
        LOG(ERROR) << "invalid font file";
        return false;
    }

    stbtt_fontinfo font;
    if (0 == stbtt_InitFont(&font, ttf, font_offset)) {
        LOG(ERROR) << "init font (building font cached) failed";
        return false;
    }

    float size_in_pixels = 20.0f;
    float scale = stbtt_ScaleForPixelHeight(&font, size_in_pixels);
    int ascent, descent, linegap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &linegap);

    Graph g;

    float offset_x = 0;
    float offset_y = 0;
    std::vector<Polygon2> contours;
    for (int i = 0; i < text.length(); i++) {
        std::vector<Polygon2> char_contours;
        if (extract_contours(&font, text[i], char_contours, 4, offset_x, offset_y))
            contours.insert(contours.end(), char_contours.begin(), char_contours.end());

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, text[i], &advanceWidth, &leftSideBearing);
        offset_x += advanceWidth - leftSideBearing;
    }

    std::size_t offset_index = 0;
    for (const auto& contour : contours) {
        for (const auto& p : contour)
            g.add_vertex(vec3(p, 0.0));
        for (std::size_t j = 0; j < contour.size(); ++j)
            g.add_edge(Graph::Vertex(offset_index + j), Graph::Vertex(offset_index + (j + 1) % contour.size()));

        offset_index += contour.size();
    }

    GraphIO::save("contours.ply", &g);

    return 0;
}
