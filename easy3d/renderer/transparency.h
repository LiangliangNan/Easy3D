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

#ifndef EASY3D_RENDERER_TRANSPARENCY_H
#define EASY3D_RENDERER_TRANSPARENCY_H

#ifdef EASY3D_HAS_VULKAN

#include <functional>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>


namespace easy3d {

    class Camera;
    class TrianglesDrawable;

    /**
     * \brief Base class for rendering with transparency.
     * \class Transparency easy3d/renderer/transparency.h
     * \details This class provides an interface for rendering scenes with transparency effects.
     * \see AverageColorBlending, DualDepthPeeling
     */
    class Transparency
    {
    public:
        Transparency() = default;
        virtual ~Transparency() = default;

        /**
         * \brief Renders the scene (a set of surfaces) with transparency effect.
         * \param surfaces The surfaces to be rendered.
         */
        virtual void draw(const std::vector<TrianglesDrawable*>& surfaces) = 0;

        // The OpenGL effects know only the camera and draw into the current
        // context. Vulkan presentation belongs to the viewer, so the viewer
        // registers a frame renderer for its camera. Method 1 is average color
        // blending and method 2 is depth peeling; the last argument bounds the
        // peel layers (zero means no explicit bound).
        using FrameRenderer = std::function<void(const std::vector<TrianglesDrawable*>&, int, int)>;
        static void set_frame_renderer(const Camera* camera, FrameRenderer renderer) {
            if (renderer) renderers()[camera] = std::move(renderer);
            else renderers().erase(camera);
        }

    protected:
        static void render_frame(const Camera* camera, const std::vector<TrianglesDrawable*>& surfaces,
                                 int method, int peels) {
            auto it = renderers().find(camera);
            if (it == renderers().end())
                throw std::logic_error("transparency requires a Viewer or OffScreen frame");
            it->second(surfaces, method, peels);
        }

    private:
        static std::map<const Camera*, FrameRenderer>& renderers() {
            static std::map<const Camera*, FrameRenderer> value;
            return value;
        }
    };

}

#else

#include <vector>


namespace easy3d {

    class Camera;
    class TrianglesDrawable;

    /**
     * \brief Base class for rendering with transparency.
     * \class Transparency easy3d/renderer/transparency.h
     * \details This class provides an interface for rendering scenes with transparency effects.
     * \see AverageColorBlending, DualDepthPeeling
     */
    class Transparency
    {
    public:
        /**
         * \brief Default constructor for Transparency.
         */
        Transparency() = default;
        /**
         * \brief Virtual destructor for Transparency.
         */
        virtual ~Transparency() = default;

        /**
         * \brief Renders the scene (a set of surfaces) with transparency effect.
         * \param surfaces The surfaces to be rendered.
         */
        virtual void draw(const std::vector<TrianglesDrawable*>& surfaces) = 0;
    };

}


#endif // EASY3D_HAS_VULKAN

#endif // EASY3D_RENDERER_TRANSPARENCY_H
