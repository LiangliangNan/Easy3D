// SPDX-License-Identifier: GPL-3.0-or-later
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
struct ColoredPoint { vec3 position, color; };
using PointBatch = std::vector<ColoredPoint>;

// UI thread only: mutate scene state and invalidate drawable snapshots together.
inline void append_point_batch(PointCloud& cloud, const PointBatch& batch) {
    auto colors=cloud.vertex_property<vec3>("v:color");
    for (const auto& point:batch) colors[cloud.add_vertex(point.position)]=point.color;
    if (cloud.renderer()) cloud.renderer()->update();
}

// Worker-owned computation, with a single immutable batch waiting for the UI.
// No Model, Renderer, Viewer or graphics API is accessed on the worker thread.
class PointBatchWorker {
public:
    explicit PointBatchWorker(unsigned batches=66,
                             std::chrono::milliseconds interval=std::chrono::milliseconds(300))
        : batches_(batches), interval_(interval) {
        if (interval.count()<0) throw std::invalid_argument("negative point batch interval");
        thread_=std::thread([this] { produce(); });
    }
    ~PointBatchWorker() { stop(); }
    PointBatchWorker(const PointBatchWorker&)=delete;
    PointBatchWorker& operator=(const PointBatchWorker&)=delete;

    // Nonblocking UI-thread consumption; the returned batch owns its storage.
    std::shared_ptr<const PointBatch> take() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (error_) std::rethrow_exception(error_);
        auto result=std::move(pending_);
        ready_.notify_all();
        return result;
    }
    // Startup/tests only: ordinary rendering uses take() without waiting.
    bool wait_for_batch(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        ready_.wait_for(lock,timeout,[this] { return pending_ || finished_ || stopping_ || error_; });
        if (error_) std::rethrow_exception(error_);
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
        { std::lock_guard<std::mutex> lock(mutex_); stopping_=true; }
        ready_.notify_all();
        if (thread_.joinable()) thread_.join();
    }
private:
    void produce() noexcept {
        try {
            std::mt19937 random(0x312);
            std::uniform_real_distribution<float> unit(0,1);
            for (unsigned i=0;i<batches_;++i) {
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    ready_.wait(lock,[this] { return stopping_ || !pending_; });
                    if (stopping_) break;
                }
                auto batch=std::make_shared<PointBatch>();
                batch->reserve(100);
                for (unsigned j=0;j<100;++j) {
                    // Draw components separately for reproducible ordering in C++11.
                    const float x=unit(random), y=unit(random), z=unit(random);
                    const float r=unit(random), g=unit(random), b=unit(random);
                    batch->push_back({vec3(x,y,z),vec3(r,g,b)});
                }
                std::unique_lock<std::mutex> lock(mutex_);
                if (stopping_) break;
                pending_=std::move(batch);
                ready_.notify_all();
                if (i+1<batches_ && ready_.wait_for(lock,interval_,[this] { return stopping_; })) break;
            }
        } catch (...) {
            std::lock_guard<std::mutex> lock(mutex_);
            error_=std::current_exception();
        }
        { std::lock_guard<std::mutex> lock(mutex_); finished_=true; }
        ready_.notify_all();
    }
    unsigned batches_;
    std::chrono::milliseconds interval_;
    mutable std::mutex mutex_;
    std::condition_variable ready_;
    std::shared_ptr<const PointBatch> pending_;
    std::exception_ptr error_;
    bool stopping_=false, finished_=false;
    std::thread thread_;
};
}
#endif
