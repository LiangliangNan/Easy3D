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


#include <easy3d/util/progress.h>

#include <cassert>
#include <algorithm>


namespace easy3d {

    //  \cond
    namespace details {

        class Progress {
        public:
            static Progress* instance();

            virtual void notify(std::size_t new_val, bool show_text = true, bool update_viewer = true);

            void set_client(ProgressClient *c) { client_ = c; }

            void push();
            void pop();

            void cancel() { canceled_ = true; }
            void clear_canceled() { canceled_ = false; }
            bool is_canceled() const { return canceled_; }

        protected:
            Progress() : client_(nullptr), level_(0), canceled_(false) {}

            virtual ~Progress() {}

            ProgressClient *client_;
            int level_;
            bool canceled_;
        };

        Progress* Progress::instance() {
            static Progress instance;
            return &instance;
        }

        void Progress::push() {
            level_++;
            if (level_ == 1) {
                clear_canceled();
            }
        }

        void Progress::pop() {
            assert(level_ > 0);
            level_--;
        }

        void Progress::notify(std::size_t new_val, bool show_text, bool update_viewer) {
            if (client_ != nullptr && level_ < 2)
                client_->notify(new_val, show_text, update_viewer);
        }
    }
    //  \endcond

    //_________________________________________________________


    ProgressClient::ProgressClient() {
        details::Progress::instance()->set_client(this);
    }

    void ProgressClient::cancel() {
        details::Progress::instance()->cancel();
    }

    //_________________________________________________________


    ProgressLogger::ProgressLogger(std::size_t max_val, const std::string &task_name, bool quiet)
            : max_val_(max_val), task_name_(task_name), quiet_(quiet) {
        cur_val_ = 0;
        cur_percent_ = 0;
        details::Progress::instance()->push();
        if (!quiet_) {
            details::Progress::instance()->notify(0, true);
        }
    }

    void ProgressLogger::reset(std::size_t max_val, bool show_text) {
        max_val_ = max_val;
        reset(show_text);
    }

    ProgressLogger::~ProgressLogger() {
        details::Progress::instance()->pop();
    }

    void ProgressLogger::next(bool update_viewer) {
        cur_val_++;
        update(true, update_viewer);
    }

    bool ProgressLogger::is_canceled() const {
        return details::Progress::instance()->is_canceled();
    }

    void ProgressLogger::notify(std::size_t new_value, bool show_text, bool update_viewer) {
        cur_val_ = new_value;
        update(show_text, update_viewer);
    }


    void ProgressLogger::update(bool show_text, bool update_viewer) {
        std::size_t percent = cur_val_ * 100 / std::max<std::size_t>(1, max_val_ - 1);
        if (percent != cur_percent_) {
            cur_percent_ = percent;
            if (!quiet_) {
                details::Progress::instance()->notify(std::min<std::size_t>(cur_percent_, 100), show_text, update_viewer);
            }
        }
    }

}