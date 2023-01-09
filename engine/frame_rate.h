
#pragma once

#include <chrono>

namespace lve {

    /**
     * todo: add a Profiler, see https://github.com/vblanco20-1/vulkan-guide/blob/engine/extra-engine/vk_profiler.h
     */

    class FrameRate {
        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
    public:
        FrameRate() : _lastFpsUpdateTime{ TimePoint::clock::now() } {}
        ~FrameRate() = default;

        /// called for each frame
        void check();

        uint32_t get() const { return _currentFPS; }

    private:
        TimePoint _lastFpsUpdateTime;
        uint32_t _frameCounter{0};
        uint32_t _currentFPS{0};
        static constexpr float _fpsUpdateIntervalMillis{1000};
    };


    inline void FrameRate::check() {
        ++_frameCounter;

        auto now = TimePoint::clock::now();
        auto duration = std::chrono::duration<float, std::milli>(now - _lastFpsUpdateTime).count();
        if (duration > _fpsUpdateIntervalMillis) {
            _currentFPS = static_cast<uint32_t>(static_cast<float>(_frameCounter) / duration * 1000.0f);
            _lastFpsUpdateTime = now;
            _frameCounter = 0;
            printf("%.1d fps\n", _currentFPS);
        }
    }

}    // lvk
