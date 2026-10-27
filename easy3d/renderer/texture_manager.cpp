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

    std::mutex TextureManager::mutex_;
    std::unordered_map<std::string, Texture *>    TextureManager::textures_;
    std::unordered_map<std::string, bool>        TextureManager::attempt_load_texture_; // avoid multiple attempt


    Texture* TextureManager::request_white() {
        std::lock_guard<std::mutex> lock(mutex_);
        const std::string name = "<easy3d:white-fallback>";
        auto found = textures_.find(name);
        if (found != textures_.end())
            return found->second;

        const std::vector<unsigned char> rgba(4, 255);
        Texture* texture = Texture::create(rgba, 1, 1, 4, Texture::CLAMP_TO_EDGE, Texture::NEAREST);
        if (texture)
            textures_[name] = texture;
        return texture;
    }


    Texture* TextureManager::request(const std::string& file, Texture::WrapMode wrap, Texture::FilterMode filter) {
        return request_impl(file,wrap,filter,false);
    }
    Texture* TextureManager::request_cpu(const std::string& file, Texture::WrapMode wrap, Texture::FilterMode filter) {
        return request_impl(file,wrap,filter,true);
    }
    Texture* TextureManager::request_impl(const std::string& file, Texture::WrapMode wrap, Texture::FilterMode filter, bool cpu) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto key = std::string(cpu ? "cpu|" : "default|")+file+"|"+std::to_string(int(wrap))+"|"+std::to_string(int(filter));
        auto found = textures_.find(key);
        if (found != textures_.end()) return found->second;
        auto attempted = attempt_load_texture_.find(key);
        if (attempted != attempt_load_texture_.end() && !attempted->second) return nullptr;
        Texture* texture = cpu ? Texture::create_cpu(file,wrap,filter) : Texture::create(file,wrap,filter);
        attempt_load_texture_[key] = texture != nullptr;
        if (texture) textures_[key] = texture;
        return texture;
    }


    Texture* TextureManager::request(const std::string &file_name, int num_stripes, Texture::WrapMode wrap, Texture::FilterMode filter) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!file_system::is_file(file_name)) {
            LOG(ERROR) << "file does not exist: " << file_name;
            return nullptr;
        }

        const std::string texture_name = file_name + "|" + std::to_string(num_stripes) + "|" + std::to_string(int(wrap)) + "|" + std::to_string(int(filter));
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
                LOG_N_TIMES(3, ERROR) << "failed to create texture from " << file_name << ". " << COUNTER;
                attempt_load_texture_[file_name] = false;
                return nullptr;
            }

            texture->name_ = texture_name;
            textures_[texture_name] = texture;
            return texture;
        }
    }


    Texture* TextureManager::request(int num_stripes, int stride, Texture::WrapMode wrap, Texture::FilterMode filter) {
        std::lock_guard<std::mutex> lock(mutex_);
        const std::string texture_name = "random|" + std::to_string(num_stripes) + "|" + std::to_string(stride) + "|" + std::to_string(int(wrap)) + "|" + std::to_string(int(filter));
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
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = textures_.begin(); it != textures_.end(); ++it) {
            if (it->second == texture) {
                attempt_load_texture_.erase(it->first);
                textures_.erase(it);
                delete texture;
                return;
            }
        }
    }


    void TextureManager::terminate() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& p : textures_)
            delete p.second;
        textures_.clear();
        attempt_load_texture_.clear();
    }

} // namespace easy3d
