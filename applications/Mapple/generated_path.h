#ifndef EASY3D_MAPPLE_GENERATED_PATH_H
#define EASY3D_MAPPLE_GENERATED_PATH_H

#include <algorithm>
#include <cstddef>
#include <vector>

#include <easy3d/renderer/camera_path_builder.h>

namespace mapple {

    struct GeneratedPath {
        easy3d::CameraPathBuilder::Source source;
        easy3d::CameraPathBuilder::Settings built_settings;
        int first = 0;
        int last = -1;
        // For each manual gap, the number of generated poses before it in path order.
        std::vector<int> manual_before;
        bool pending = true;

        int span() const { return last >= first ? last - first + 1 : 0; }
        int size() const { return span() - static_cast<int>(manual_before.size()); }
        int manual_position(std::size_t i) const {
            return first + manual_before[i] + static_cast<int>(i);
        }

        int generated_before(int index) const {
            int count = index - first;
            for (std::size_t i = 0; i < manual_before.size(); ++i) {
                if (manual_position(i) >= index)
                    break;
                --count;
            }
            return count;
        }

        void keyframe_inserted(int index) {
            if (span() == 0) {
                if (index < first) {
                    ++first;
                    ++last;
                } else if (index == first) {
                    manual_before.push_back(0);
                    last = first;
                }
                return;
            }
            if (index <= first) {
                ++first;
                ++last;
            } else if (index <= last) {
                manual_before.push_back(generated_before(index));
                std::sort(manual_before.begin(), manual_before.end());
                ++last;
            }
        }

        void keyframe_removed(int index) {
            if (span() == 0) {
                if (index < first) {
                    --first;
                    --last;
                }
                return;
            }
            if (index < first) {
                --first;
                --last;
                return;
            }
            if (index > last)
                return;

            for (std::size_t i = 0; i < manual_before.size(); ++i) {
                if (manual_position(i) == index) {
                    manual_before.erase(manual_before.begin() + static_cast<std::ptrdiff_t>(i));
                    --last;
                    return;
                }
            }

            const int generated = generated_before(index);
            for (int &anchor : manual_before) {
                if (anchor > generated)
                    --anchor;
            }
            --last;
        }

        bool stale(const easy3d::CameraPathBuilder &builder) const {
            return pending || builder.settings_changed_for(source, built_settings);
        }
    };

} // namespace mapple

#endif
