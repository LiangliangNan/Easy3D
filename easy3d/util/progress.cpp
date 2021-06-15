/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/


#include <easy3d/util/progress.h>

#include <cassert>
#include <algorithm>


namespace easy3d {

    //  \cond
    namespace details {

        class Progress {
        public:
            static Progress* instance();

            virtual void notify(std::size_t percent, bool update_viewer);

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

        void Progress::notify(std::size_t percent, bool update_viewer) {
            if (client_ != nullptr && level_ < 2)
                client_->notify(percent, update_viewer);
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


    ProgressLogger::ProgressLogger(std::size_t max_val, bool update_viewer, bool quiet)
            : max_val_(max_val)
            , cur_val_(0)
            , cur_percent_(0)
            , quiet_(quiet)
            , update_viewer_(update_viewer)
    {
        details::Progress::instance()->push();
        if (!quiet_) {
            details::Progress::instance()->notify(0, update_viewer_);
        }
    }


    ProgressLogger::~ProgressLogger() {
        // one more notification to make sure the progress reaches its end
        details::Progress::instance()->notify(100, update_viewer_);
        details::Progress::instance()->pop();
    }


    void ProgressLogger::notify(std::size_t new_value) {
        cur_val_ = new_value;
        update();
    }


    void ProgressLogger::next() {
        cur_val_++;
        update();
    }


    bool ProgressLogger::is_canceled() const {
        return details::Progress::instance()->is_canceled();
    }


    void ProgressLogger::reset(std::size_t max_val) {
        max_val_ = max_val;
        reset();
    }


    void ProgressLogger::update() {
        std::size_t percent = cur_val_ * 100 / std::max<std::size_t>(1, max_val_ - 1);
        if (percent != cur_percent_) {
            cur_percent_ = percent;
            if (!quiet_) {
                details::Progress::instance()->notify(std::min<std::size_t>(cur_percent_, 100), update_viewer_);
            }
        }
    }

}