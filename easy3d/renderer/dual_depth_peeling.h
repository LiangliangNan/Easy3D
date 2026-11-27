/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_RENDERER_DUAL_DEPTH_PEELING_H
#define EASY3D_RENDERER_DUAL_DEPTH_PEELING_H

#include <easy3d/core/types.h>

#ifdef EASY3D_HAS_VULKAN

#include <vector>
#include <easy3d/renderer/transparency.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    class Camera;
    class TrianglesDrawable;

    /**
     * \brief Transparency effect using front-to-back depth peeling.
     * \details Both backends keep the nearest surviving transparent fragment per
     *      peel-target sample and layer, then under-blend the premultiplied layers
     *      over the opaque scene. The legacy name is retained for source compatibility;
     *      neither backend uses the historical min-max front/back pairing.
     * \class DualDepthPeeling easy3d/renderer/dual_depth_peeling.h
     * \sa Transparency, AverageColorBlending, PerPixelLinkedListsOIT
     */
    class EASY3D_DEPRECATED("use RenderSettings transparency controls and portable graphics::shape commands with RenderDevice") DualDepthPeeling : public Transparency
    {
    public:
        /**
         * \brief Constructor
         * \param cam The camera used in the view
         */
        explicit DualDepthPeeling(Camera* cam) : camera_(cam) {
            LOG(INFO) << "maximum number of peeling layers: " << max_peels_;
        }
        /**
         * \brief Destructor
         */
        ~DualDepthPeeling() override = default;

        /**
         * \brief Sets the maximum number of peeling layers.
         * \details Defaults to 4; 0 requests all layers, capped at 16 by the portable backends.
         *      Given a complex object and a particular view, the theoretical ideal number equals to its actual depth
         *      layers in that view.
         * \param n The maximum number of peeling layers.
         */
        void set_max_peels(int n) {
            max_peels_ = n;
            LOG(INFO) << "maximum number of peeling layers: " << max_peels_;
        }
        /**
         * \brief Returns the maximum number of peeling layers.
         * \return The maximum number of peeling layers
         */
        int max_peels() const { return max_peels_; }

        /**
         * \brief Renders the scene (a set of surfaces) with transparency effect
         * \details Routes to the viewer's registered frame renderer (method 2), passing
         *      max_peels_ as the bound on the peeled layers.
         * \param surfaces The surfaces to be rendered
         */
        void draw(const std::vector<TrianglesDrawable*>& surfaces) override {
            render_frame(camera_, surfaces, 2, max_peels_);
        }

    private:
        // The camera whose scene is rendered.
        Camera* camera_;
        // Maximum number of peeling layers (zero uses the portable backend cap).
        int max_peels_ = 4;
    };

}

#else

#include <vector>
#include <easy3d/renderer/transparency.h>
#include <easy3d/util/logging.h>

namespace easy3d {

    class Camera;
    class TrianglesDrawable;

    /**
     * \brief Transparency effect using front-to-back depth peeling.
     * \details Keeps the nearest surviving transparent fragment per peel-target
     *      sample and layer, then under-blends the premultiplied layers over the
     *      opaque scene, matching the portable Vulkan and OpenGL RenderDevice paths.
     * \class DualDepthPeeling easy3d/renderer/dual_depth_peeling.h
     * \sa Transparency, AverageColorBlending, PerPixelLinkedListsOIT
     */
    class EASY3D_DEPRECATED("use RenderSettings transparency controls and portable graphics::shape commands with RenderDevice") DualDepthPeeling : public Transparency
    {
    public:
        /**
         * \brief Constructor
         * \param cam The camera used in the view
         */
        explicit DualDepthPeeling(Camera* cam);
        /**
         * \brief Destructor
         */
        ~DualDepthPeeling() override;

        /**
         * \brief Sets the maximum number of peeling layers.
         * \details Defaults to 4; 0 requests all layers, capped at 16 by the portable backends.
         *      Given a complex object and a particular view, the theoretical ideal number equals to its actual depth
         *      layers in that view.
         * \param n The maximum number of peeling layers.
         */
        void set_max_peels(int n) {
            max_peels_ = n;
            LOG(INFO) << "maximum number of peeling layers: " << max_peels_;
        }

        /**
         * \brief Returns the maximum number of peeling layers.
         * \return The maximum number of peeling layers
         */
        int  max_peels() const { return max_peels_; }

        /**
         * \brief Renders the scene (a set of surfaces) with transparency effect
         * \param surfaces The surfaces to be rendered
         */
        void draw(const std::vector<TrianglesDrawable*>& surfaces) override;

    protected:

        // Allocate or resize the layer and accumulation targets.
        void init(int w, int h);

        // Clear the previous-depth sentinels and accumulation target.
        void init_depth_buffers(const std::vector<TrianglesDrawable*>& surfaces);

        // Draw the nearest surviving fragments into the current layer.
        void geometry_pass(const std::vector<TrianglesDrawable*>& surfaces);

        // Resolve one layer and under-blend it into the accumulation target.
        void peel(const std::vector<TrianglesDrawable*>& surfaces);

        // Composite premultiplied accumulation over the current framebuffer.
        void blend_final_image();

        // Exchange the current and previous layer targets.
        void swap_targets();

        // Check the layer bound and whether the last layer was empty.
        bool peeling_done() const;

        // Create the query used to detect an empty layer.
        void init_occlusion_query();
        // Destroy the layer query.
        void delete_occlusion_query_id();
        // Begin counting fragments in the layer composite.
        void start_occlusion_query() const;
        // Finish the query and store the fragment count.
        void end_occlusion_query();

        // Release the framebuffer targets.
        void clear();

    private:
        /* Methods to block default compiler methods.
         * The compiler automatically generates the following methods.
         *  Since the default compiler implementation is generally not what
         *  you want (for all but the most simple classes), we usually
         *  put the declarations of these methods in the private section
         *  and never implement them. This prevents the compiler from
         *  implementing an incorrect "default" behavior without us
         *  knowing. (See Scott Meyers book, "Effective C++")
         */
        //copying disabled
        DualDepthPeeling(const DualDepthPeeling&);
        DualDepthPeeling& operator=(const DualDepthPeeling&);

        // The camera whose scene is rendered.
        Camera* camera_;

        // Current viewport size.
        int width_;
        int height_;

        // Defines the maximum number of peeling layers. Initial value is 4.
        // Zero requests all layers, capped at 16 by the portable backends.
        int max_peels_;

        // Retain the historical stage and target layout for binary compatibility.
        enum DualDepthPeelingStage {
            DDP_InitDepth,
            DDP_Peel,
            DDP_Blend,
            DDP_Inactive = -1,
        };
        DualDepthPeelingStage stage_;

        // Opaque implementation storage keeps the legacy class layout stable.
        struct Resources;
        Resources* resources_;

        enum TextureName {
            DepthA = 0,
            DepthB,
            BackTemp,
            Back,
            FrontA,
            FrontB
        };
        TextureName front_source_;
        TextureName front_destination_;
        TextureName depth_source_;
        TextureName depth_destination_;

        // Peel progress and debug pass counter.
        int current_peel_;
        int num_geom_passes_;

        // Occlusion-query state used for early termination of the peeling loop.
        bool use_occlusion_query_;
        unsigned int occlusion_query_Id_;
        unsigned int num_written_pixels_;
        unsigned int occlusion_threshold_;

        // Saved clear color, restored after the final blend.
        float bkg_color_[4];
    };

}


#endif // EASY3D_HAS_VULKAN

#endif // EASY3D_RENDERER_DUAL_DEPTH_PEELING_H
