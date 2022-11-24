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

#include <easy3d/renderer/texture_manager.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/core/random.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>


namespace easy3d {

    std::unordered_map<std::string, Texture *>    TextureManager::textures_;
    std::unordered_map<std::string, bool>        TextureManager::attempt_load_texture_; // avoid multiple attempt


    Texture* TextureManager::request(const std::string &image_file, Texture::WrapMode wrap, Texture::FilterMode filter) {
        auto pos = textures_.find(image_file);
        if (pos != textures_.end()) // program already exists
            return pos->second;
        else {
            auto it = attempt_load_texture_.find(image_file);
            if (it == attempt_load_texture_.end())
                attempt_load_texture_[image_file] = true;
            else if (!attempt_load_texture_[image_file])
                return nullptr;

            Texture *texture = Texture::create(image_file, wrap, filter);
            if (!texture) {
                LOG_N_TIMES(3, ERROR) << "failed creating texture from image file: " << image_file
                                      << ". " << COUNTER;
                attempt_load_texture_[image_file] = false;
                return nullptr;
            }

            textures_[image_file] = texture;
            return texture;
        }
    }


    Texture* TextureManager::request(const std::string &file_name, int num_stripes, Texture::WrapMode wrap, Texture::FilterMode filter) {
        if (!file_system::is_file(file_name)) {
            LOG(ERROR) << "file does not exist: " << file_name;
            return nullptr;
        }

        const std::string texture_name = file_name + "|" + std::to_string(num_stripes);
        auto pos = textures_.find(texture_name);
        if (pos != textures_.end()) // program already exists
            return pos->second;
        else {
            auto it = attempt_load_texture_.find(texture_name);
            if (it == attempt_load_texture_.end())
                attempt_load_texture_[texture_name] = true;
            else if (!attempt_load_texture_[texture_name])
                return nullptr;

            int width = 0, height = 0, comp = 0;
            std::vector<unsigned char> data;
            // flip the image vertically, so the first pixel in the output array is the bottom left
            bool success = ImageIO::load(file_name, data, width, height, comp, 0, true);
            if (!success || data.empty())
                return nullptr;

            discretize_image(data, width, height, comp, num_stripes);

            Texture *texture = Texture::create(data, width, height, comp, wrap, filter);
            if (!texture) {
                LOG_N_TIMES(3, ERROR) << "failed creating texture from image file: " << file_name
                                      << ". " << COUNTER;
                attempt_load_texture_[file_name] = false;
                return nullptr;
            }

            texture->name_ = texture_name;
            textures_[texture_name] = texture;
            return texture;
        }
    }


    Texture* TextureManager::request(int num_stripes, int stride, Texture::WrapMode wrap, Texture::FilterMode filter) {
        const std::string texture_name = "random|" + std::to_string(num_stripes) + "|" + std::to_string(stride);
        auto pos = textures_.find(texture_name);
        if (pos != textures_.end()) // program already exists
            return pos->second;
        else {
            std::vector<unsigned char> data(num_stripes * stride * 3);
            for (int x = 0; x < num_stripes; ++x) {
                const vec3 &c = random_color(false);
                for (int j = 0; j < stride; ++j) {
                    const int base = (x * stride + j) * 3;
                    data[base]     = static_cast<unsigned char>(c.r * 255.0f);
                    data[base + 1] = static_cast<unsigned char>(c.g * 255.0f);
                    data[base + 2] = static_cast<unsigned char>(c.b * 255.0f);
                }
            }

            Texture *texture = Texture::create(data, num_stripes * stride, 1, 3, wrap, filter);
            if (!texture) {
                LOG(ERROR) << "failed creating texture from image data";
                return nullptr;
            }

            // though random colored texture, just give a name
            texture->name_ = texture_name;
            textures_[texture->name_] = texture;

            return texture;
        }
    }

    void TextureManager::release(const Texture* texture) {
        for (const auto& p : textures_) {
            if (p.second == texture) {
                textures_.erase(texture->name());
                attempt_load_texture_.erase(texture->name());
                delete texture;
                return;
            }
        }
    }


    void TextureManager::terminate() {
        for (const auto& p : textures_)
            delete p.second;
        textures_.clear();
        attempt_load_texture_.clear();
    }

} // namespace easy3d