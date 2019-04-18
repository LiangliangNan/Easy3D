/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_SOFT_SHADOW_H
#define EASY3D_SOFT_SHADOW_H

#include <easy3d/viewer/shadow.h>


namespace easy3d {

    /*
     * SoftShadow implements Percentage-Closer Soft Shadows - by Randima Fernando
     * See http://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
     * https://http.download.nvidia.com/developer/presentations/2005/SIGGRAPH/Percentage_Closer_Soft_Shadows.pdf
     *
     * --- Overview ---
     * This sample demonstrates two techniques for rendering soft shadows: Percentage Closer Filtering (PCS) with
     * a uniform kernel size and Percentage-Closer Soft Shadows (PCSS) with a variable kernel size.
     * This sample is assuming the shadow map is a regular depth buffer (non-linear depths), and the shadow-map
     * is generated using a perspective projection with the center of projection at the center of the area light
     * simulated by PCSS. The sample uses a gradient-based depth bias to scale the depth bias based on the uv-space
     * distance from the center of the kernel. To avoid self-shadowing artifacts, an additional depth bias may also
     * been applied while writing depth into the shadow map.
     *
     * --- Percentage Closer Filtering (PCF) ---
     * The Percentage Closer Filtering technique uses a uniform size filter kernel, resulting in shadows with a
     * uniformly soft edge. The PCF algorithm is very straightforward and fast, but does not achieve a realistic
     * contact hardening effect. Also, while the kernel size and penumbra width is uniform, the kernel can be
     * scaled to accomodate different sized lights.
     *
     * --- Percentage-Closer Soft Shadows (PCSS) ---
     * Soft shadows are typically rendered in games by using shadow mapping and Percentage Closer Filtering with
     * a uniform kernel size. The Percentage-Closer Soft Shadows (PCSS) algorithm computes a variable kernel size
     * based on the distance between the relative position of the receiver point, an approximation of the blocker,
     * and the area light. It produces perceptually plausible soft shadows that harden on contact.
     *
     * Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO) can benefit from sharing the same
     * geometry pass.
     */

    class SoftShadow : public Shadow
    {
    public:
        enum SamplePattern {
            SP_Poisson_25_25 = 0,   // 25 samples for the blocker search, 25 samples in the PCF filtering, all samples using a Poisson disk
            SP_Poisson_32_64 = 1,   // 32 samples for the blocker search, 64 samples for the PCF filtering, all samples using a Poisson disk
            SP_Poisson_64_128 = 2,  // 64 samples for the blocker search, 128 samples for the PCF filtering, all samples using a Poisson disk
            SP_Poisson_100_100 = 3, // 100 samples for the blocker search, 100 samples for the PCF filtering, all samples using a Poisson disk
            SP_Regular_49_225 = 4   // 49 samples for the blocker search, 225 samples for the PCF filtering, all samples using regular sampling (faster than Poisson disk)
        };

    public:
        SoftShadow(Camera* cam);
        virtual ~SoftShadow() override;

        // The softness of the shadow. Values must be in [0, 1]. Default: 0.5.
        float softness() const { return softness_; }
        void set_softness(float s) { softness_ = s; }

        // The sampler pattern used for blocker search and PCF filtering.
        SamplePattern sample_pattern() const { return sample_pattern_; }
        void set_sample_pattern(SamplePattern pattern) { sample_pattern_ = pattern; }

    protected:
        virtual void ensure_fbo() override;
        virtual void shadow_map_pass(const std::vector<TrianglesDrawable*>& surfaces) override;
        virtual void render_pass(const std::vector<TrianglesDrawable*>& surfaces) override;

    protected:
        // The softness of the shadow, in [0, 1] (w.r.t. 10% of the light's size)
        float softness_;

        SamplePattern sample_pattern_;

    private:
        //copying disabled
        SoftShadow(const SoftShadow&);
        SoftShadow& operator=(const SoftShadow&);
    };

}

#endif // EASY3D_SOFT_SHADOW_H
