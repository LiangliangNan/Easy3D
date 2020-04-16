/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/viewer/texture_manager.h>
#include <easy3d/core/random.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    std::unordered_map<std::string, Texture *>    TextureManager::textures_;
    std::unordered_map<std::string, bool>        TextureManager::attempt_load_texture_; // avoid multiple attempt

    Texture* TextureManager::request(const std::string &image_file, Texture::WrapMode wrap, Texture::FilterMode filter) {
        std::unordered_map<std::string, Texture *>::iterator pos = textures_.find(image_file);
        if (pos != textures_.end()) // program already exists
            return pos->second;
        else {
            std::unordered_map<std::string, bool>::iterator it = attempt_load_texture_.find(image_file);
            if (it == attempt_load_texture_.end())
                attempt_load_texture_[image_file] = true;
            else if (!attempt_load_texture_[image_file])
                return nullptr;

            Texture *texture = Texture::create(image_file, wrap, filter);
            if (!texture) {
                LOG_FIRST_N(ERROR, 1) << "failed creating texture from image file: " << image_file
                                      << " (this is the first record)";
                attempt_load_texture_[image_file] = false;
                return nullptr;
            }

            textures_[image_file] = texture;
            return texture;
        }
    }


    Texture* TextureManager::request(int num_colors, Texture::WrapMode wrap, Texture::FilterMode filter) {
        std::vector<unsigned char> data(num_colors * 3);
        for(int x=0; x<num_colors; ++x) {
            const vec3& c = random_color(false);
            data[x * 3] = c.r * 255;
            data[x * 3 + 1] = c.g * 255;
            data[x * 3 + 2] = c.b * 255;
        }

        Texture *texture = Texture::create(data, num_colors, 1, 3, wrap, filter);
        if (!texture) {
            LOG(ERROR) << "failed creating texture from image data";
            return nullptr;
        }

        const std::string& texture_id = std::to_string(texture->id());
        textures_[texture_id] = texture;

        LOG(INFO) << "a random-color texture generated, with id: " << texture->id();
        return texture;
    }

    void TextureManager::release(const Texture* texture) {
        for (auto p : textures_) {
            if (p.second == texture) {
                textures_.erase(texture->file_name());
                attempt_load_texture_.erase(texture->file_name());
                delete texture;
                return;
            }
        }
    }


    void TextureManager::terminate() {
        for (auto p : textures_)
            delete p.second;
        textures_.clear();
        attempt_load_texture_.clear();
    }

} // namespace easy3d