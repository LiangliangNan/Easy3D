/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#ifndef EASY3D_UTIL_PROGRESS_H
#define EASY3D_UTIL_PROGRESS_H


#include <string>


namespace easy3d {

    /**
     * \brief The base class of GUI element reporting the progress.
     * \class ProgressClient easy3d/util/progress.h
     */
    class ProgressClient {
    public:
        /**
         * \brief Constructor.
         */
        ProgressClient();
        /**
         * \brief Destructor.
         */
        virtual ~ProgressClient() = default;
        /**
         * \brief Notify the progress.
         * \param percent The progress percentage.
         * \param update_viewer True to update the viewer.
         */
        virtual void notify(std::size_t percent, bool update_viewer) = 0;
        /**
         * \brief Cancel the progress.
         */
        virtual void cancel();
    };

    //_________________________________________________________

    /**
     * \brief An implementation of progress logging mechanism.
     * \class ProgressLogger easy3d/util/progress.h
     */
    class ProgressLogger {
    public:
        /**
         * \brief Constructor.
         * \param max_val The max value (i.e., upper bound) of the progress range.
         * \param update_viewer True to trigger the viewer to update for each step.
         * \param quiet True to make the logger quiet (i.e., don't notify the client).
         */
        ProgressLogger(std::size_t max_val, bool update_viewer, bool quiet = false);
        /**
         * \brief Destructor.
         */
        virtual ~ProgressLogger();
        /**
         * \brief Notify the progress.
         * \param new_value The new progress value.
         */
        virtual void notify(std::size_t new_value);
        /**
         * \brief Move to the next step.
         */
        virtual void next();
        /**
         * \brief Mark the progress as done.
         */
        virtual void done() { notify(max_val_); }
        /**
         * \brief Check if the progress is canceled.
         * \return True if the progress is canceled, false otherwise.
         */
        bool is_canceled() const;

        /**
         * \brief Resets the progress logger without changing the progress range.
         */
        void reset() { notify(0); }
        /**
         * \brief Resets the progress logger, and meanwhile changes the progress range.
         * \param max_val The new max value of the progress range.
         */
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

    /**
     * \brief A simple progress indicator for console applications.
     * \param percentage The progress percentage.
     * \details Given percentage = 0.75, the output looks like:
     *     75% [||||||||||||||||||||||||||||||||||||||||||               ]
     */
    void print_progress(float percentage);

}   // namespace easy3d


#endif  // EASY3D_UTIL_PROGRESS_H

