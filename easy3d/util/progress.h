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

#ifndef EASY3D_UTIL_PROGRESS_H
#define EASY3D_UTIL_PROGRESS_H


#include <string>


namespace easy3d {

    /**
     * \brief The based class of GUI element reporting the progress.
     * \class ProgressClient easy3d/util/progress.h
     */
    class ProgressClient {
    public:
        ProgressClient();
        virtual ~ProgressClient() {}
        virtual void notify(std::size_t new_value, bool show_text = true, bool update_viewer = true) = 0;
        virtual void cancel();
    };

    //_________________________________________________________

    /**
     * \brief An implementation of progress logging mechanism.
     * \class ProgressLogger easy3d/util/progress.h
     */
    class ProgressLogger {
    public:
        ProgressLogger(std::size_t max_val = 100, const std::string &task_name = "", bool quiet = false);
        virtual ~ProgressLogger();

        virtual void notify(std::size_t new_value, bool show_text = true, bool update_viewer = true);
        virtual void next(bool update_viewer = true);
        virtual void done() { notify(max_val_); }

        bool is_canceled() const;

        void reset(bool show_text = true) { notify(0, show_text); }
        void reset(std::size_t max_val, bool show_text = true);

    protected:
        virtual void update(bool show_text, bool update_viewer);

    private:
        std::size_t max_val_;
        std::string task_name_;
        std::size_t cur_val_;
        std::size_t cur_percent_;
        bool quiet_;
    };

}   // namespace easy3d


#endif  // EASY3D_UTIL_PROGRESS_H

