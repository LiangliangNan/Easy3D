/** \cond LICENSE
 * SPDX-License-Identifier: GPL-3.0-or-later
 \endcond */
#ifndef EASY3D_TUTORIAL_POINT_WORKER_H
#define EASY3D_TUTORIAL_POINT_WORKER_H
#include <easy3d/core/types.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/renderer.h>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace easy3d {
    /// A plain CPU data record: one point position plus its display color. No graphics state.
    struct ColoredPoint {
        vec3 position, color;
    };
    /// One immutable batch of points handed from the worker to the UI thread.
    using PointBatch = std::vector<ColoredPoint>;

    // UI thread only: mutate scene state and invalidate drawable snapshots together.
    // This is the consumer's application step: it runs on the viewer thread (from the
    // animation callback), never on the worker, because PointCloud and Renderer are not
    // thread-safe and the GPU buffers must be refreshed from the thread that owns them.
    inline void append_point_batch(PointCloud &cloud, const PointBatch &batch) {
        auto colors = cloud.vertex_property<vec3>("v:color");
        for (const auto &point : batch)
            colors[cloud.add_vertex(point.position)] = point.color;
        // The cloud grew: rebuild the drawables' CPU/GPU snapshots so the new points render.
        if (cloud.renderer())
            cloud.renderer()->update();
    }

    /**
     * \brief A background producer of point batches with a single-slot, thread-safe handoff.
     *
     * The worker thread generates random colored points at a fixed pace and publishes at most
     * one batch at a time; the UI thread consumes batches with take(). A mutex + condition
     * variable protect the slot, backpressure blocks the worker while a batch is unconsumed,
     * and stop()/the destructor guarantee the thread is joined before owner state dies.
     */
    // Worker-owned computation, with a single immutable batch waiting for the UI.
    // No Model, Renderer, Viewer or graphics API is accessed on the worker thread.
    class PointBatchWorker {
    public:
        // Configure the number of batches and the pacing interval, then start the producer
        // thread. The thread captures `this`, so the object must outlive it — stop() in the
        // destructor enforces exactly that.
        explicit PointBatchWorker(unsigned batches = 66,
                                  std::chrono::milliseconds interval = std::chrono::milliseconds(300))
            : batches_(batches), interval_(interval) {
            if (interval.count() < 0)
                throw std::invalid_argument("negative point batch interval");
            thread_ = std::thread([this] { produce(); });
        }
        // Always interrupt and join the worker before any member state is destroyed.
        ~PointBatchWorker() {
            stop();
        }
        // The worker owns a thread and a mutex: copying it would be meaningless and unsafe.
        PointBatchWorker(const PointBatchWorker &) = delete;
        PointBatchWorker &operator=(const PointBatchWorker &) = delete;

        // Nonblocking UI-thread consumption; the returned batch owns its storage.
        // Moving the shared_ptr out of the slot transfers ownership to the caller and frees
        // the slot, which wakes the worker to produce the next batch.
        std::shared_ptr<const PointBatch> take() {
            std::lock_guard<std::mutex> lock(mutex_);
            // Surface any worker failure on the UI thread instead of losing it silently.
            if (error_)
                std::rethrow_exception(error_);
            auto result = std::move(pending_);
            ready_.notify_all();
            return result;
        }
        // Startup/tests only: ordinary rendering uses take() without waiting.
        // Blocks up to `timeout` for the first publication (or shutdown/error).
        bool wait_for_batch(std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(mutex_);
            ready_.wait_for(lock, timeout, [this] { return pending_ || finished_ || stopping_ || error_; });
            if (error_)
                std::rethrow_exception(error_);
            return bool(pending_);
        }
        // Check completion and pending storage under the same lock, so a final
        // publication cannot race with the UI deciding to pause consumption.
        bool drained() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return finished_ && !pending_;
        }
        // Called by the owner/UI thread. Interrupts both pacing and backpressure
        // waits, and joins before any worker state or viewer resources can die.
        void stop() {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopping_ = true;
            }
            // Wake every wait so the worker observes stopping_ promptly, then join.
            ready_.notify_all();
            if (thread_.joinable())
                thread_.join();
        }

    private:
        // The producer loop, running entirely on the worker thread. It only touches plain
        // CPU data and the shared slot — never the cloud, renderer, viewer, or graphics API.
        void produce() noexcept {
            try {
                // Fixed seed: the demo is reproducible run to run.
                std::mt19937 random(0x312);
                std::uniform_real_distribution<float> unit(0, 1);
                for (unsigned i = 0; i < batches_; ++i) {
                    {
                        // Backpressure: wait until the slot is empty (or we are told to stop).
                        // This bounds memory to one pending batch if the UI stops consuming.
                        std::unique_lock<std::mutex> lock(mutex_);
                        ready_.wait(lock, [this] { return stopping_ || !pending_; });
                        if (stopping_)
                            break;
                    }
                    // Generate the batch OUTSIDE the lock: computation never blocks the UI.
                    auto batch = std::make_shared<PointBatch>();
                    batch->reserve(100);
                    for (unsigned j = 0; j < 100; ++j) {
                        // Draw components separately for reproducible ordering in C++11.
                        const float x = unit(random), y = unit(random), z = unit(random);
                        const float r = unit(random), g = unit(random), b = unit(random);
                        batch->push_back({vec3(x, y, z), vec3(r, g, b)});
                    }
                    // Publish: hand the immutable batch to the slot and wake the consumer.
                    std::unique_lock<std::mutex> lock(mutex_);
                    if (stopping_)
                        break;
                    pending_ = std::move(batch);
                    ready_.notify_all();
                    // Pace the production; the timed wait doubles as an interruptible sleep
                    // (returns early with true if stopping_ was set).
                    if (i + 1 < batches_ && ready_.wait_for(lock, interval_, [this] { return stopping_; }))
                        break;
                }
            } catch (...) {
                // Exceptions must not escape a thread entry point; store and rethrow on the UI side.
                std::lock_guard<std::mutex> lock(mutex_);
                error_ = std::current_exception();
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                finished_ = true;
            }
            ready_.notify_all();
        }
        unsigned batches_;                          //!< How many batches to produce in total.
        std::chrono::milliseconds interval_;        //!< Pause between successive batches.
        mutable std::mutex mutex_;                  //!< Guards all shared state below (mutable for drained()).
        std::condition_variable ready_;             //!< Signals slot changes, completion, and shutdown.
        std::shared_ptr<const PointBatch> pending_; //!< The single-slot handoff; const = immutable once published.
        std::exception_ptr error_;                  //!< Worker failure, rethrown on the UI thread.
        bool stopping_ = false, finished_ = false;  //!< Shutdown request / producer completion flags.
        std::thread thread_;                        //!< The producer thread; joined by stop().
    };
} // namespace easy3d
#endif
