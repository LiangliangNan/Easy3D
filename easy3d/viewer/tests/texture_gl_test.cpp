// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
    try {
        initialize(false,false,false);
        Viewer viewer("OpenGL texture compatibility"); // constructor creates a hidden GL context
        while (glGetError() != GL_NO_ERROR) {}
        const std::vector<unsigned char> rgba = {255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,0,255};
        std::unique_ptr<Texture> texture(Texture::create(rgba,2,2,4,Texture::REPEAT,Texture::NEAREST));
        require(texture && texture->id() && glIsTexture(texture->id()),"legacy texture factory did not create a GL texture");
        texture->bind();
        std::vector<unsigned char> readback(16);
        glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,readback.data());
        require(readback == rgba && texture->pixels()->bytes == rgba,"GL upload or retained CPU pixels changed orientation/data");
        GLint wrap=0,filter=0;
        glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,&wrap);
        glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&filter);
        require(wrap == GL_REPEAT && filter == GL_NEAREST,"legacy GL sampler settings changed");
        std::unique_ptr<Texture> cpu(Texture::create_cpu(rgba,2,2,4));
        bool rejected=false;
        try { cpu->bind(); } catch (const std::logic_error&) { rejected=true; }
        require(rejected && !cpu->id(),"CPU texture unexpectedly acquired a GL realization");
        require(ImageIO::save("opengl-texture-source.png",rgba,2,2,4,true),"could not save GL texture fixture");
        auto* first=TextureManager::request("opengl-texture-source.png",Texture::CLAMP_TO_EDGE,Texture::NEAREST);
        auto* second=TextureManager::request("opengl-texture-source.png",Texture::REPEAT,Texture::LINEAR);
        require(first && second && first != second && first->id() != second->id(),"GL texture cache ignored sampler settings");
        second->bind();
        glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,&wrap);
        glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&filter);
        require(wrap == GL_REPEAT && filter == GL_LINEAR,"cached GL sampler mismatch");
        second->release();
        TextureManager::release(first); TextureManager::release(second);
        require(glGetError() == GL_NO_ERROR,"GL texture compatibility test reported a GL error");
        std::cout << "PASS: OpenGL texture upload/readback, CPU snapshots, samplers and cache lifecycle\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
