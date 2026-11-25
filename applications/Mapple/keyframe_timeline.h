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
 * Easy3D is distributed in the hope that it will be useful in your
 * research/work, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */

#ifndef EASY3D_MAPPLE_KEYFRAME_TIMELINE_H
#define EASY3D_MAPPLE_KEYFRAME_TIMELINE_H

#include <easy3d/renderer/keyframe_interpolator.h>

#include <vector>


/**
 * \brief Mapple's ImGui timeline strip for a KeyframeInterpolator path.
 * \class KeyframeTimeline applications/Mapple/keyframe_timeline.h
 *
 * \details Draws a horizontal timeline strip showing each keyframe as a marker
 *      and each travel/dwell segment as a bar proportional to its duration.
 *      Segments with dwell time are drawn in a different color than pure travel
 *      segments. Clicking on the strip selects the corresponding keyframe.
 *
 *      This is Mapple UI code, rather than a renderer API. It deliberately
 *      remains private to the application and is linked only into Mapple.
 */
class KeyframeTimeline
{
public:
    /**
     * \brief Creates a timeline bound to \p kfi.
     * \param kfi The keyframe interpolator to visualize. Must outlive this object.
     */
    explicit KeyframeTimeline(easy3d::KeyframeInterpolator* kfi);

    /**
     * \brief Draws the timeline strip and returns true if the selection changed.
     * \param width The available width in logical pixels.
     * \param selected_keyframe Optional in/out selected keyframe index. The
     *      selected marker is highlighted and a click replaces this index.
     * \return True when a click changed the selection.
     */
    bool draw(float width, int* selected_keyframe = nullptr);

    /**
     * \brief Marks the cached layout stale after a keyframe edit.
     * \details The strip rebuilds its segments when it next draws. Call this after
     *      anything that changes a keyframe's time or dwell time. Adding or removing
     *      a keyframe needs no call: the strip notices the new count itself.
     */
    void invalidate() { segments_dirty_ = true; }

    /**
     * \brief Returns the total timeline duration in seconds (travel + dwell).
     * \details This is the path's duration as the strip lays it out, i.e. what the
     *      recorded video lasts at an interpolation speed of 1.0.
     */
    float total_duration() const;

private:
    // One entry per timeline segment: dwell at keyframe i, then travel to i+1.
    struct Segment {
        int keyframe_index;   // keyframe at the start of this segment
        bool is_dwell;        // true when this segment is a dwell pause
        float start;          // start time on the timeline (seconds)
        float end;            // end time on the timeline (seconds)
        float duration;       // end - start
    };

    void build_segments();
    bool cache_is_current() const;
    int find_keyframe_at(float x, float width) const;

    easy3d::KeyframeInterpolator* kfi_;
    std::vector<Segment> segments_;
    std::vector<float> keyframe_times_;
    float total_duration_ = 0.0f;
    mutable bool segments_dirty_ = true;
};

#endif // EASY3D_MAPPLE_KEYFRAME_TIMELINE_H
