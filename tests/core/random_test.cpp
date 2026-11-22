// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/random.h>
#include <algorithm>

using namespace easy3d;

int main() {
    return test::run("random and indexed colors are bright and distinct", [] {
        for (int i = 0; i < 1000; ++i) {
            const vec3 dark = random_color(true);
            test::require(dark.x >= 0.0f && dark.x <= 1.0f &&
                          dark.y >= 0.0f && dark.y <= 1.0f &&
                          dark.z >= 0.0f && dark.z <= 1.0f,
                          "dark-enabled color is outside the RGB range");

            const vec3 bright = random_color();
            const float maximum = std::max(bright.x, std::max(bright.y, bright.z));
            const float minimum = std::min(bright.x, std::min(bright.y, bright.z));
            test::require(maximum >= 0.8f && maximum <= 1.0f,
                          "default color brightness is outside [0.8, 1]");
            test::require(maximum - minimum >= 0.6f - 1e-6f,
                          "default color saturation is too low");
        }

        // The colors of indices: as bright and as saturated as the random ones, but
        // reproducible, and distinct from each other where it matters.
        for (int index = -3; index < 64; ++index) {
            const vec3 color = color_from_index(index);
            test::require(color.x >= 0.0f && color.x <= 1.0f &&
                          color.y >= 0.0f && color.y <= 1.0f &&
                          color.z >= 0.0f && color.z <= 1.0f,
                          "indexed color is outside the RGB range");
            const float maximum = std::max(color.x, std::max(color.y, color.z));
            const float minimum = std::min(color.x, std::min(color.y, color.z));
            test::require(maximum >= 0.8f && maximum <= 1.0f,
                          "indexed color brightness is outside [0.8, 1]");
            test::require(maximum - minimum >= 0.6f - 1e-6f,
                          "indexed color saturation is too low");
            // An index is not a draw: the color must not depend on the generator state,
            // nor on how often it was asked for.
            test::require(color == color_from_index(index), "indexed color is not reproducible");
        }
        test::require(color_from_index(0) != color_from_index(1), "consecutive indices share a color");
        test::require(color_from_index(0) == hsv_to_rgb(0.0f, 0.75f, 0.9f),
                      "the color of index 0 is not the wheel's red");

        // The closest pair of the first 16 colors is 0.14 apart in RGB, and the first 8
        // are 0.26 apart (measured): the golden angle keeps them well apart for a table
        // of this size. The guarantee does not stretch to the first 64 (0.03 apart), so
        // nothing above a few dozen indices is claimed here.
        for (int count = 4; count <= 16; count += 4) {
            const auto table = color_table(count);
            test::require(table.size() == static_cast<std::size_t>(count),
                          "color table does not have the requested number of colors");
            for (int i = 0; i < count; ++i) {
                test::require(table[i] == color_from_index(i),
                              "color table entry is not the color of its index");
                for (int j = i + 1; j < count; ++j) {
                    const float separation = distance(table[i], table[j]);
                    test::require(separation >= 0.1f, "colors of a small table are not distinct");
                }
            }
        }
        // A count is a signed index range here, and nothing is asked for below one.
        test::require(color_table(0).empty(), "the table of no colors is not empty");
        test::require(color_table(-1).empty(), "a negative count is not an empty table");
    });
}
