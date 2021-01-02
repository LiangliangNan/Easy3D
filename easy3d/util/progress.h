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
        virtual void notify(std::size_t percent, bool update_viewer) = 0;
        virtual void cancel();
    };

    //_________________________________________________________

    /**
     * \brief An implementation of progress logging mechanism.
     * \class ProgressLogger easy3d/util/progress.h
     */
    class ProgressLogger {
    public:
        /// \param max_val The max value (i.e., upper bound) of the progress range.
        /// \param update_viewer \c true to trigger the viewer to update for each step.
        /// \param quiet \c true to make the logger quiet (i.e., don't notify the client).
        ProgressLogger(std::size_t max_val, bool update_viewer, bool quiet = false);
        virtual ~ProgressLogger();

        virtual void notify(std::size_t new_value);
        virtual void next();
        virtual void done() { notify(max_val_); }

        bool is_canceled() const;

        /// Resets the progress logger without changing the progress range.
        void reset() { notify(0); }
        /// Resets the progress logger, and meanwhile changes the progress range.
        void reset(std::size_t max_val);

    protected:
        virtual void update();

    private:
        std::size_t max_val_;
        std::size_t cur_val_;
        std::size_t cur_percent_;
        bool quiet_;
        bool update_viewer_;
    };

}   // namespace easy3d


#endif  // EASY3D_UTIL_PROGRESS_H

