// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/offscreen.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
const std::string source = "mixed-lifecycle-source.png";
const std::vector<unsigned char> orange = {230, 90, 30, 255};
class OpenGLHost : public Viewer {
public:
    OpenGLHost() : Viewer("Lifecycle OpenGL", 4, 3, 2, false, true, 24, 8, 161, 127) {
        manual_.clear(); show_easy3d_logo_ = false; init();
    }
    GLFWwindow* handle() const { return window_; }
};
template<class Host> void scene(Host& host, Texture* texture) {
    auto mesh = std::make_shared<SurfaceMesh>();
    const auto a = mesh->add_vertex(vec3(-1, -1, 0));
    const auto b = mesh->add_vertex(vec3(1, -1, 0));
    const auto c = mesh->add_vertex(vec3(0, 1, 0));
    mesh->add_triangle(a, b, c);
    auto uv = mesh->add_vertex_property<vec2>("v:texcoord", vec2(.5f));
    (void)uv;
    require(host.add_model(mesh) == mesh.get(), "scene attachment failed");
    auto faces = mesh->renderer()->get_triangles_drawable("faces");
    faces->set_lighting(false);
    faces->set_texture_coloring(State::VERTEX, "v:texcoord", texture);
    host.camera()->setType(Camera::ORTHOGRAPHIC);
    host.camera()->setViewDirection(vec3(0, 0, -1));
    host.camera()->setUpVector(vec3(0, 1, 0));
    host.fit_screen();
}
void color(const std::vector<unsigned char>& rgba, int w, int h) {
    const auto center = (size_t(h / 2) * w + w / 2) * 4;
    require(rgba.size() == size_t(w) * h * 4, "invalid image size");
    for (unsigned i = 0; i < 3; ++i)
        require(std::abs(int(rgba[center+i]) - orange[i]) <= 2, "surviving scene lost its texture/color");
}
void check(OpenGLHost& host) {
    // snapshot() must select this host even when another GL context is current.
    require(host.snapshot("mixed-lifecycle-gl.png", 1, 4, 0), "surviving OpenGL snapshot failed");
    int w = 0, h = 0, channels = 0;
    std::vector<unsigned char> rgba;
    require(ImageIO::load("mixed-lifecycle-gl.png", rgba, w, h, channels, 4, false), "GL snapshot unreadable");
    color(rgba, w, h);
    require(glfwGetCurrentContext() == host.handle(), "snapshot used another context");
    require(glGetError() == GL_NO_ERROR, "OpenGL context/resource error after host transition");
}
void check(VulkanViewer& host) {
    bool presented = false;
    for (unsigned i = 0; i < 100 && !presented; ++i) {
        VulkanViewer::poll_events(); presented = host.draw_frame();
        if (!presented) glfwWaitEventsTimeout(.01);
    }
    require(presented, "surviving Vulkan window could not present");
    const auto frame = host.capture(4);
    color(frame.rgba, frame.width, frame.height);
    require(host.device().validation_errors() == 0, "mixed lifecycle Vulkan validation error");
}
std::unique_ptr<OpenGLHost> opengl() {
    std::unique_ptr<OpenGLHost> host(new OpenGLHost);
    auto* texture = TextureManager::request(source, Texture::CLAMP_TO_EDGE, Texture::NEAREST);
    require(texture && texture->id(), "OpenGL cache texture missing");
    scene(*host, texture); check(*host); return host;
}
std::unique_ptr<VulkanViewer> vulkan(bool validation) {
    VulkanViewerOptions options;
    options.width = 161; options.height = 127; options.visible = false; options.samples = 4;
    options.device.validation = options.device.synchronization_validation = validation;
    std::unique_ptr<VulkanViewer> host(new VulkanViewer("Lifecycle Vulkan", options));
    auto* texture = TextureManager::request_cpu(source, Texture::CLAMP_TO_EDGE, Texture::NEAREST);
    require(texture && !texture->id(), "CPU cache texture missing");
    scene(*host, texture); check(*host); return host;
}
void failed_opengl_creation(OpenGLHost& survivor) {
    bool rejected = false;
    try { Viewer invalid("Invalid context", 4, 99, 0); }
    catch (const BackendUnavailable&) { rejected = true; }
    require(rejected, "invalid OpenGL version unexpectedly succeeded");
    check(survivor);
}
void exercise(bool vulkan_first, bool validation) {
    std::unique_ptr<VulkanViewer> native;
    if (vulkan_first) native = vulkan(validation);
    auto first = opengl();
    if (!vulkan_first) native = vulkan(validation);
    auto second = opengl();
    auto* cached = TextureManager::request(source, Texture::CLAMP_TO_EDGE, Texture::NEAREST);
    auto* cpu = TextureManager::request_cpu(source, Texture::CLAMP_TO_EDGE, Texture::NEAREST);
    const auto programs = ShaderManager::all_programs();
    require(!programs.empty(), "shader cache was not populated");
    // New window creation must not reset GLFW's process-wide clock.
    const double before = glfwGetTime();
    auto third = opengl();
    require(glfwGetTime() >= before, "creating a viewer reset the toolkit clock");
    first.reset(); // the original context/share-window can disappear first
    require(glfwGetCurrentContext() == third->handle(), "cleanup did not restore the previous context");
    require(ShaderManager::all_programs() == programs, "closing one GL host cleared shared shaders");
    require(TextureManager::request(source, Texture::CLAMP_TO_EDGE, Texture::NEAREST) == cached &&
            glIsTexture(cached->id()), "closing one GL host invalidated shared textures");
    check(*second); check(*native);
    third.reset();
    auto replacement = opengl(); // share with the surviving context, not the original window
    failed_opengl_creation(*second);
    replacement.reset();
    ClippingPlane::instance()->set_enabled(true);
    second.reset(); // last GL host must leave Vulkan and its CPU texture intact
    require(ClippingPlane::instance()->is_enabled(), "closing OpenGL changed shared clipping state");
    ClippingPlane::instance()->set_enabled(false);
    require(ShaderManager::all_programs().empty(), "last GL host retained GPU shader cache");
    require(TextureManager::request_cpu(source, Texture::CLAMP_TO_EDGE, Texture::NEAREST) == cpu &&
            cpu->pixels()->bytes == orange, "OpenGL shutdown cleared the native CPU texture cache");
    check(*native);
    auto after = opengl();
    native.reset(); // terminating Vulkan first must also leave OpenGL alive
    check(*after);
    after.reset();
    TextureManager::release(cpu);
}
void unavailable_vulkan(const std::string& expected) {
    auto gl = opengl();
    for (unsigned i = 0; i < 2; ++i) {
        bool rejected = false;
        try { VulkanViewer invalid("Unavailable Vulkan"); }
        catch (const BackendUnavailable& e) {
            std::cout << "Unavailable Vulkan: " << e.what() << '\n';
            rejected = std::string(e.what()).find(expected) != std::string::npos;
        }
        require(rejected, "Vulkan request did not retain its availability diagnostic");
        OffScreenOptions automatic;
        OffScreen renderer(161, 127, automatic);
        require(renderer.backend() == Backend::OpenGL && !renderer.fallback_reason().empty(),
                "Automatic fallback failed with a live OpenGL viewer");
        scene(renderer, TextureManager::request(source, Texture::CLAMP_TO_EDGE, Texture::NEAREST));
        require(renderer.render("mixed-lifecycle-fallback.png", 1, 4, 0), "overlapping fallback renderer failed");
        check(*gl);
    }
    failed_opengl_creation(*gl);
}
}
int main(int argc, char** argv) {
    try {
        initialize(false, false, false);
        require(ImageIO::save(source, orange, 1, 1, 4, false), "texture fixture could not be saved");
        if (argc > 1 && std::string(argv[1]) == "--missing-loader") unavailable_vulkan("Vulkan loader unavailable:");
        else if (argc > 1 && std::string(argv[1]) == "--missing-driver") unavailable_vulkan("Vulkan");
        else {
            const bool validation = argc > 1;
            exercise(false, validation);
            exercise(true, validation);
        }
        std::cout << "PASS: shared GLFW lifetime, both backend orders, surviving GL caches/CPU textures, failure recovery\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
