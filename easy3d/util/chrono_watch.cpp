#include <easy3d/util/chrono_watch.h>

#include <cassert>
#include <cmath>
#include <sstream>
#include <iomanip>

#include <easy3d/core/types.h>


namespace easy3d {

    ChronoWatch::ChronoWatch()
        : started_(false)
        , paused_(false)
    {
    }

    void ChronoWatch::start() {
        assert(!started_);
        started_ = true;
        paused_ = false;
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    void ChronoWatch::pause() {
        assert(started_);
        paused_ = true;
        pause_time_ = std::chrono::high_resolution_clock::now();
    }

    void ChronoWatch::resume() {
        assert(paused_);
        paused_ = false;
        start_time_ += std::chrono::high_resolution_clock::now() - pause_time_;
    }

    void ChronoWatch::reset() {
        assert(started_ || paused_);
        started_ = false;
        paused_ = false;
        pause_time_ = start_time_ = std::chrono::high_resolution_clock::now();
    }

    std::size_t ChronoWatch::duration_count() const {
        assert(started_);
        if (paused_) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(pause_time_ - start_time_);
            return static_cast<std::size_t>(duration.count());
        }
        else {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time_);
            return static_cast<std::size_t>(duration.count());
        }
    }

    float ChronoWatch::elapsed_milliseconds(int num_digits) const {
        auto t = duration_count();
        return geom::truncate_digits(static_cast<float>(t), num_digits);
    }

    float ChronoWatch::elapsed_seconds(int num_digits) const {
        auto t = duration_count() / 1e3f;
        return geom::truncate_digits(static_cast<float>(t), num_digits);
    }

    float ChronoWatch::elapsed_minutes(int num_digits) const {
        auto t = duration_count() / 1e3f / 60.0f;
        return geom::truncate_digits(static_cast<float>(t), num_digits);
    }

    float ChronoWatch::elapsed_hours(int num_digits) const {
        auto t = duration_count() / 1e3f / 60.0f / 60.0f;
        return geom::truncate_digits(static_cast<float>(t), num_digits);
    }

    void ChronoWatch::print_seconds(std::ostream& output) const {
        output << "Elapsed time: " << elapsed_seconds() << " seconds." << std::endl;
    }

    void ChronoWatch::print_minutes(std::ostream& output) const {
        output << "Elapsed time: " << elapsed_minutes() << " minutes." << std::endl;
    }

    void ChronoWatch::print_hours(std::ostream& output) const {
        output << "Elapsed time: " << elapsed_hours() << " hours." << std::endl;
    }

    std::string ChronoWatch::time_string(int num_digits /* = 1*/) const {
        double time = static_cast<double>(duration_count());
        if (std::isnan(time) || std::isinf(time))
            return "inf";

        std::string suffix = "ms";
        if (time > 1000) {
            time /= 1000; suffix = "s";
            if (time > 60) {
                time /= 60; suffix = "m";
                if (time > 60) {
                    time /= 60; suffix = "h";
                    if (time > 12) {
                        time /= 12; suffix = "d";
                    }
                }
            }
        }

        std::ostringstream os;
        os << std::setprecision(num_digits)
            << std::fixed << time << suffix;

        return os.str();
    }

} // namespace easy3d
