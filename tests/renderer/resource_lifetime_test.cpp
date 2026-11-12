// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/render_device.h>
#include <cmath>
#include <iostream>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace easy3d;
using namespace easy3d::graphics;

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

template <typename T> struct ReusedStorage {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;

    template <typename... Args> std::shared_ptr<const T> make(Args &&...args) {
        T *value = new (&storage) T(std::forward<Args>(args)...);
        return std::shared_ptr<const T>(value, [](const T *object) { const_cast<T *>(object)->~T(); });
    }
};

std::vector<Vertex> triangle(float x_offset) {
    std::vector<Vertex> vertices(3);
    vertices[0].position = vec3(-.8f + x_offset, -.8f, 0);
    vertices[1].position = vec3(.8f + x_offset, -.8f, 0);
    vertices[2].position = vec3(x_offset, .8f, 0);
    return vertices;
}

void check_reused_resources(RenderDevice &device) {
    ReusedStorage<Image> image_storage;
    DrawCommand draw;
    draw.geometry = std::make_shared<Geometry>(triangle(0), std::vector<uint32_t>{0, 1, 2});
    draw.texture = image_storage.make(1, 1, std::vector<unsigned char>{255, 0, 0, 255});
    draw.object_id = 71;
    draw.color = vec4(1);

    const auto red = device.render(33, 33, {draw}, vec4(0));
    const size_t center = (16 * 33 + 16) * 4;
    require(red.rgba[center] == 255 && red.rgba[center + 2] == 0, "initial texture upload failed");
    const auto pixel = device.sample_pixel(33, 33, {draw}, 16, 16);
    require(pixel.id.object == draw.object_id && std::abs(pixel.depth - .5f) < 1e-5f,
            "synchronous pixel capture returned the wrong scene sample");

    draw.texture.reset();
    draw.texture = image_storage.make(1, 1, std::vector<unsigned char>{0, 0, 255, 255});
    const auto blue = device.render(33, 33, {draw}, vec4(0));
    require(blue.rgba[center] == 0 && blue.rgba[center + 2] == 255,
            "reused texture address retained the previous GPU image");

    ReusedStorage<Geometry> geometry_storage;
    draw.texture.reset();
    draw.geometry.reset();
    draw.geometry = geometry_storage.make(triangle(0), std::vector<uint32_t>{0, 1, 2});
    const auto centered = device.render(33, 33, {draw}, vec4(0));
    require(centered.ids[16 * 33 + 16].object == draw.object_id, "initial geometry upload failed");

    draw.geometry.reset();
    draw.geometry = geometry_storage.make(triangle(1.5f), std::vector<uint32_t>{0, 1, 2});
    const auto shifted = device.render(33, 33, {draw}, vec4(0));
    require(shifted.ids[16 * 33 + 16].object == UINT32_MAX,
            "reused geometry address retained the previous GPU buffer");

    const auto resized = device.render(17, 19, {}, vec4(0));
    require(resized.width == 17 && resized.height == 19, "capture target resize failed");
}

int main(int argc, char **) {
    try {
        const bool validation = argc > 1;
        for (unsigned cycle = 0; cycle < 2; ++cycle) {
            auto device = create_vulkan_device(validation, validation);
            check_reused_resources(*device);
            require(device->validation_errors() == 0, "resource lifetime or synchronization validation errors");
        }
        std::cout << "PASS: GPU cache lifetime, synchronous image and pixel readback, target resize, and device teardown\n";
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
