// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/random.h>
#include <algorithm>

using namespace easy3d;

int main() {
    return test::run("random colors are bright and saturated", [] {
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
    });
}
