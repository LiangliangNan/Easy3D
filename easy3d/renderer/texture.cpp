/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include <easy3d/renderer/texture.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace easy3d {
Texture::Texture() : id_(0), wrap_mode_(CLAMP_TO_EDGE), filter_mode_(LINEAR) {
    sizes_[0] = sizes_[1] = sizes_[2] = 0;
}
Texture::~Texture() {
#ifdef EASY3D_RENDERER_OPENGL
    if (id_) opengl_destroy();
#endif
}
Texture* Texture::create_cpu(const std::vector<unsigned char>& data, int width, int height, int comp,
                             WrapMode wrap, FilterMode filter) {
    if (width <= 0 || height <= 0 || comp < 1 || comp > 4 ||
        uint64_t(width)*height > std::numeric_limits<size_t>::max()/unsigned(comp) ||
        data.size() != uint64_t(width)*height*unsigned(comp))
        throw std::invalid_argument("image data does not match positive dimensions/channel count");
    if ((wrap != CLAMP_TO_EDGE && wrap != REPEAT) || (filter != NEAREST && filter != LINEAR))
        throw std::invalid_argument("invalid texture sampler");
    std::unique_ptr<Texture> texture(new Texture);
    std::shared_ptr<PixelData> pixels(new PixelData{width,height,comp,data});
    texture->pixels_ = pixels;
    texture->sizes_[0] = width; texture->sizes_[1] = height; texture->sizes_[2] = comp;
    texture->wrap_mode_ = wrap; texture->filter_mode_ = filter;
    return texture.release();
}
Texture* Texture::create_cpu(const std::string& file, WrapMode wrap, FilterMode filter) {
    int width=0,height=0,comp=0;
    std::vector<unsigned char> data;
    if (!ImageIO::load(file,data,width,height,comp,0,true) || data.empty()) return nullptr;
    auto* texture = create_cpu(data,width,height,comp,wrap,filter);
    texture->name_ = file;
    return texture;
}
Texture* Texture::create(const std::vector<unsigned char>& data, int width, int height, int comp,
                         WrapMode wrap, FilterMode filter) {
    try {
        std::unique_ptr<Texture> texture(create_cpu(data,width,height,comp,wrap,filter));
#ifdef EASY3D_RENDERER_OPENGL
        if (!texture->opengl_upload()) return nullptr;
#endif
        return texture.release();
    } catch (const std::invalid_argument& e) {
        LOG(ERROR) << e.what();
        return nullptr;
    }
}
Texture* Texture::create(const std::string& file, WrapMode wrap, FilterMode filter) {
    int width=0,height=0,comp=0;
    std::vector<unsigned char> data;
    if (!ImageIO::load(file,data,width,height,comp,0,true) || data.empty()) return nullptr;
    auto* texture = create(data,width,height,comp,wrap,filter);
    if (texture) texture->name_ = file;
    return texture;
}
#ifndef EASY3D_RENDERER_OPENGL
void Texture::bind(int) const { throw std::logic_error("OpenGL texture binding is unavailable"); }
void Texture::release() const { throw std::logic_error("OpenGL texture binding is unavailable"); }
#endif

    void discretize_image(std::vector<unsigned char> &data, int width, int height, int channels, int num_stripes) {
        if (num_stripes >= width)
            return;

        const int stride = width / num_stripes;
        for (int i = 0; i < num_stripes; ++i) {
            for (int j = 0; j < height; ++j) {
                for (int m = 0; m < stride; ++m)
                    for (int k = 0; k < channels; ++k)
                        data[j * (width * channels) + i * (stride * channels) + m * channels + k] =
                                data[j * (width * channels) + i * stride * channels + k];
            }
        }
    }


} // namespace easy3d
