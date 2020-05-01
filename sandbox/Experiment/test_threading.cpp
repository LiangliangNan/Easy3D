// Copyright (c) 2018, ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: Johannes L. Schoenberger (jsch-at-demuc-dot-de)

#include <easy3d/util/logging.h>
#include <easy3d/util/threading.h>

using namespace easy3d;

void test_thread_wait() {
    class TestThread : public Thread {
        void Run() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
    };

    TestThread thread;
    CHECK_TRUE(!thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Start();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Wait();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(thread.IsFinished());
}

void test_thread_pause() {
    class TestThread : public Thread {
        void Run() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            BlockIfPaused();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    };

    TestThread thread;
    CHECK_TRUE(!thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Start();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Wait();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(thread.IsFinished());
}

void test_thread_stop() {
    class TestThread : public Thread {
        void Run() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if (IsStopped()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return;
            }
        }
    };

    TestThread thread;
    CHECK_TRUE(!thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Start();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Wait();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(thread.IsFinished());
}

void test_thread_pause_stop() {
    class TestThread : public Thread {
        void Run() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            BlockIfPaused();
            if (IsStopped()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                return;
            }
        }
    };

    TestThread thread;
    CHECK_TRUE(!thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Start();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Wait();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(thread.IsFinished());
}

void test_thread_restart() {
    class TestThread : public Thread {
        void Run() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
    };

    TestThread thread;
    CHECK_TRUE(!thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    for (size_t i = 0; i < 2; ++i) {
        thread.Start();
        CHECK_TRUE(thread.IsStarted());
        CHECK_TRUE(!thread.IsStopped());
        CHECK_TRUE(!thread.IsPaused());
        CHECK_TRUE(thread.IsRunning());
        CHECK_TRUE(!thread.IsFinished());

        thread.Wait();
        CHECK_TRUE(thread.IsStarted());
        CHECK_TRUE(!thread.IsStopped());
        CHECK_TRUE(!thread.IsPaused());
        CHECK_TRUE(!thread.IsRunning());
        CHECK_TRUE(thread.IsFinished());
    }
}

void test_thread_valid_setup() {
    class TestThread : public Thread {
        void Run() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            SignalValidSetup();
        }
    };

    TestThread thread;
    CHECK_TRUE(!thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Start();

    CHECK_TRUE(thread.CheckValidSetup());
    CHECK_TRUE(thread.CheckValidSetup());

    thread.Wait();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(thread.IsFinished());
    CHECK_TRUE(thread.CheckValidSetup());
}

void test_thread_invalid_setup() {
    class TestThread : public Thread {
        void Run() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            SignalInvalidSetup();
        }
    };

    TestThread thread;
    CHECK_TRUE(!thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(!thread.IsFinished());

    thread.Start();

    CHECK_TRUE(!thread.CheckValidSetup());
    CHECK_TRUE(!thread.CheckValidSetup());

    thread.Wait();
    CHECK_TRUE(thread.IsStarted());
    CHECK_TRUE(!thread.IsStopped());
    CHECK_TRUE(!thread.IsPaused());
    CHECK_TRUE(!thread.IsRunning());
    CHECK_TRUE(thread.IsFinished());
    CHECK_TRUE(!thread.CheckValidSetup());
}

void test_thread_test_callback() {
    class TestThread : public Thread {
    public:
        enum Callbacks {
            CALLBACK1,
            CALLBACK2,
        };

        TestThread() {
            RegisterCallback(CALLBACK1);
            RegisterCallback(CALLBACK2);
        }

    private:
        void Run() {
            Callback(CALLBACK1);
            Callback(CALLBACK2);
        }
    };

    bool called_back1 = false;
    std::function<void()> CallbackFunc1 = [&called_back1]() {
        called_back1 = true;
    };

    bool called_back2 = false;
    std::function<void()> CallbackFunc2 = [&called_back2]() {
        called_back2 = true;
    };

    bool called_back3 = false;
    std::function<void()> CallbackFunc3 = [&called_back3]() {
        called_back3 = true;
    };

    TestThread thread;
    thread.AddCallback(TestThread::CALLBACK1, CallbackFunc1);
    thread.Start();
    thread.Wait();
    CHECK_TRUE(called_back1);
    CHECK_TRUE(!called_back2);
    CHECK_TRUE(!called_back3);

    called_back1 = false;
    called_back2 = false;
    thread.AddCallback(TestThread::CALLBACK2, CallbackFunc2);
    thread.Start();
    thread.Wait();
    CHECK_TRUE(called_back1);
    CHECK_TRUE(called_back2);
    CHECK_TRUE(!called_back3);

    called_back1 = false;
    called_back2 = false;
    called_back3 = false;
    thread.AddCallback(TestThread::CALLBACK1, CallbackFunc3);
    thread.Start();
    thread.Wait();
    CHECK_TRUE(called_back1);
    CHECK_TRUE(called_back2);
    CHECK_TRUE(called_back3);
}

void test_thread_test_default_callback() {
    class TestThread : public Thread {
    private:
        void Run() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); }
    };

    bool called_back1 = false;
    std::function<void()> CallbackFunc1 = [&called_back1]() {
        called_back1 = true;
    };

    bool called_back2 = false;
    std::function<void()> CallbackFunc2 = [&called_back2]() {
        called_back2 = true;
    };

    TestThread thread;
    thread.AddCallback(TestThread::STARTED_CALLBACK, CallbackFunc1);
    thread.AddCallback(TestThread::FINISHED_CALLBACK, CallbackFunc2);
    thread.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK_TRUE(called_back1);
    CHECK_TRUE(!called_back2);
    thread.Wait();
    CHECK_TRUE(called_back1);
    CHECK_TRUE(called_back2);
}

void test_thread_timer() {
    class TestThread : public Thread {
        void Run() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            BlockIfPaused();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    };

    TestThread thread;
    thread.Start();
    thread.Wait();
    const auto elapsed_seconds1 = thread.GetTimer().elapsed_seconds();
    CHECK_GT(elapsed_seconds1, 0.35);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK_EQ(thread.GetTimer().elapsed_seconds(), elapsed_seconds1);

    thread.Start();
    CHECK_LT(thread.GetTimer().elapsed_seconds(), elapsed_seconds1);

    thread.Pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    const auto elapsed_seconds2 = thread.GetTimer().elapsed_seconds();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK_EQ(thread.GetTimer().elapsed_seconds(), elapsed_seconds2);

    thread.Resume();
    thread.Wait();
    CHECK_GT(thread.GetTimer().elapsed_seconds(), elapsed_seconds2);
    CHECK_GT(thread.GetTimer().elapsed_seconds(), 0.35);
}

void test_thread_pool_no_arg_no_return() {
    std::function<void(void)> Func = []() {
        int num = 0;
        for (int i = 0; i < 1000; ++i) {
            num += i;
        }
    };

    ThreadPool pool(4);
    std::vector<std::future<void>> futures;

    for (int i = 0; i < 1000; ++i) {
        futures.push_back(pool.AddTask(Func));
    }

    for (auto& future : futures) {
        future.get();
    }
}

void test_thread_pool_arg_no_return() {
    std::function<void(int)> Func = [](int num) {
        for (int i = 0; i < 1000; ++i) {
            num += i;
        }
    };

    ThreadPool pool(4);
    std::vector<std::future<void>> futures;

    for (int i = 0; i < 1000; ++i) {
        futures.push_back(pool.AddTask(Func, i));
    }

    for (auto& future : futures) {
        future.get();
    }
}

void test_thread_pool_no_arg_return() {
    std::function<int(void)> Func = []() { return 0; };

    ThreadPool pool(4);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 1000; ++i) {
        futures.push_back(pool.AddTask(Func));
    }

    for (auto& future : futures) {
        future.get();
    }
}

void test_thread_pool_arg_return() {
    std::function<int(int)> Func = [](int num) {
        for (int i = 0; i < 1000; ++i) {
            num += i;
        }
        return num;
    };

    ThreadPool pool(4);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 1000; ++i) {
        futures.push_back(pool.AddTask(Func, i));
    }

    for (auto& future : futures) {
        future.get();
    }
}

void test_thread_pool_destructor() {
    std::vector<uint8_t> results(1000, 0);
    std::function<void(int)> Func = [&results](const int num) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        results[num] = 1;
    };

    {
        ThreadPool pool(4);
        for (size_t i = 0; i < results.size(); ++i) {
            pool.AddTask(Func, i);
        }
    }

    bool missing_result = false;
    for (const auto result : results) {
        if (result == 0) {
            missing_result = true;
            break;
        }
    }

    CHECK_TRUE(missing_result);
}

void test_thread_pool_stop() {
    std::function<int(int)> Func = [](int num) {
        for (int i = 0; i < 1000; ++i) {
            num += i;
        }
        return num;
    };

    ThreadPool pool(4);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 1000; ++i) {
        futures.push_back(pool.AddTask(Func, i));
    }

    pool.Stop();

//  BOOST_CHECK_THROW(pool.AddTask(Func, 1000), std::runtime_error);

    pool.Stop();
}

void test_thread_pool_wait() {
    std::vector<uint8_t> results(1000, 0);
    std::function<void(int)> Func = [&results](const int num) {
        results[num] = 1;
    };

    ThreadPool pool(4);
    pool.Wait();

    for (size_t i = 0; i < results.size(); ++i) {
        pool.AddTask(Func, i);
    }

    pool.Wait();

    for (const auto result : results) {
        CHECK_EQ(result, 1);
    }
}

void test_thread_pool_wait_with_pause() {
    std::vector<uint8_t> results(4, 0);
    std::function<void(int)> Func = [&results](const int num) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        results[num] = 1;
    };

    ThreadPool pool(4);

    for (size_t i = 0; i < results.size(); ++i) {
        pool.AddTask(Func, i);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pool.Wait();

    for (const auto result : results) {
        CHECK_EQ(result, 1);
    }
}

void test_thread_pool_wait_without_pause() {
    std::vector<uint8_t> results(4, 0);
    std::function<void(int)> Func = [&results](const int num) {
        results[num] = 1;
    };

    ThreadPool pool(4);

    for (size_t i = 0; i < results.size(); ++i) {
        pool.AddTask(Func, i);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.Wait();

    for (const auto result : results) {
        CHECK_EQ(result, 1);
    }
}

void test_thread_pool_wait_everytime() {
    std::vector<uint8_t> results(4, 0);
    std::function<void(int)> Func = [&results](const int num) {
        results[num] = 1;
    };

    ThreadPool pool(4);

    for (size_t i = 0; i < results.size(); ++i) {
        pool.AddTask(Func, i);
        pool.Wait();

        for (size_t j = 0; j < results.size(); ++j) {
            if (j <= i) {
                CHECK_EQ(results[j], 1);
            } else {
                CHECK_EQ(results[j], 0);
            }
        }
    }

    pool.Wait();
}

void test_thread_pool_get_thread_index() {
    ThreadPool pool(4);

    std::vector<int> results(1000, -1);
    std::function<void(int)> Func = [&](const int num) {
        results[num] = pool.GetThreadIndex();
    };

    for (size_t i = 0; i < results.size(); ++i) {
        pool.AddTask(Func, i);
    }

    pool.Wait();

    for (const auto result : results) {
        CHECK_GE(result, 0);
        CHECK_LE(result, 3);
    }
}

void test_thread_test_job_queue_single_producer_single_consumer() {
    JobQueue<int> job_queue;

    std::thread producer_thread([&job_queue]() {
        for (int i = 0; i < 10; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread consumer_thread([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_EQ(job_queue.Size(), 10);
        for (int i = 0; i < 10; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_EQ(job.Data(), i);
        }
    });

    producer_thread.join();
    consumer_thread.join();
}

void test_thread_test_job_queue_single_producer_single_consumer_max_num_jobs() {
    JobQueue<int> job_queue(2);

    std::thread producer_thread([&job_queue]() {
        for (int i = 0; i < 10; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread consumer_thread([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_EQ(job_queue.Size(), 2);
        for (int i = 0; i < 10; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_EQ(job.Data(), i);
        }
    });

    producer_thread.join();
    consumer_thread.join();
}

void test_thread_test_job_queue_multiple_producer_single_consumer() {
    JobQueue<int> job_queue(1);

    std::thread producer_thread1([&job_queue]() {
        for (int i = 0; i < 10; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread producer_thread2([&job_queue]() {
        for (int i = 0; i < 10; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread consumer_thread([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_EQ(job_queue.Size(), 1);
        for (int i = 0; i < 20; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_LT(job.Data(), 10);
        }
    });

    producer_thread1.join();
    producer_thread2.join();
    consumer_thread.join();
}

void test_thread_test_job_queue_single_producer_multiple_consumer() {
    JobQueue<int> job_queue(1);

    std::thread producer_thread([&job_queue]() {
        for (int i = 0; i < 20; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread consumer_thread1([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_LE(job_queue.Size(), 1);
        for (int i = 0; i < 10; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_LT(job.Data(), 20);
        }
    });

    std::thread consumer_thread2([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_LE(job_queue.Size(), 1);
        for (int i = 0; i < 10; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_LT(job.Data(), 20);
        }
    });

    producer_thread.join();
    consumer_thread1.join();
    consumer_thread2.join();
}

void test_thread_test_job_queue_multiple_producer_multiple_consumer() {
    JobQueue<int> job_queue(1);

    std::thread producer_thread1([&job_queue]() {
        for (int i = 0; i < 10; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread producer_thread2([&job_queue]() {
        for (int i = 0; i < 10; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread consumer_thread1([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_LE(job_queue.Size(), 1);
        for (int i = 0; i < 10; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_LT(job.Data(), 10);
        }
    });

    std::thread consumer_thread2([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_LE(job_queue.Size(), 1);
        for (int i = 0; i < 10; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_LT(job.Data(), 10);
        }
    });

    producer_thread1.join();
    producer_thread2.join();
    consumer_thread1.join();
    consumer_thread2.join();
}

void test_thread_test_job_queue_wait() {
    JobQueue<int> job_queue;

    std::thread producer_thread([&job_queue]() {
        for (int i = 0; i < 10; ++i) {
            CHECK(job_queue.Push(i));
        }
    });

    std::thread consumer_thread([&job_queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_EQ(job_queue.Size(), 10);
        for (int i = 0; i < 10; ++i) {
            const auto job = job_queue.Pop();
            CHECK(job.IsValid());
            CHECK_EQ(job.Data(), i);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    job_queue.Wait();

    CHECK_EQ(job_queue.Size(), 0);
    CHECK_TRUE(job_queue.Push(0));
    CHECK_TRUE(job_queue.Pop().IsValid());

    producer_thread.join();
    consumer_thread.join();
}

void test_thread_test_job_queue_stop_producer() {
    JobQueue<int> job_queue(1);

    std::thread producer_thread([&job_queue]() {
        CHECK(job_queue.Push(0));
        CHECK(!job_queue.Push(0));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK_EQ(job_queue.Size(), 1);

    job_queue.Stop();
    producer_thread.join();

    CHECK_TRUE(!job_queue.Push(0));
    CHECK_TRUE(!job_queue.Pop().IsValid());
}

void test_thread_test_job_queue_stop_consumer() {
    JobQueue<int> job_queue(1);

    CHECK_TRUE(job_queue.Push(0));

    std::thread consumer_thread([&job_queue]() {
        const auto job = job_queue.Pop();
        CHECK(job.IsValid());
        CHECK_EQ(job.Data(), 0);
        CHECK(!job_queue.Pop().IsValid());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK_EQ(job_queue.Size(), 0);

    job_queue.Stop();
    consumer_thread.join();

    CHECK_TRUE(!job_queue.Push(0));
    CHECK_TRUE(!job_queue.Pop().IsValid());
}

void test_thread_test_job_queue_clear() {
    JobQueue<int> job_queue(1);

    CHECK_TRUE(job_queue.Push(0));
    CHECK_EQ(job_queue.Size(), 1);

    job_queue.Clear();
    CHECK_EQ(job_queue.Size(), 0);
}

void test_thread_test_get_effective_num_threads() {
    CHECK_GT(GetEffectiveNumThreads(-2), 0);
    CHECK_GT(GetEffectiveNumThreads(-1), 0);
    CHECK_GT(GetEffectiveNumThreads(0), 0);
    CHECK_EQ(GetEffectiveNumThreads(1), 1);
    CHECK_EQ(GetEffectiveNumThreads(2), 2);
    CHECK_EQ(GetEffectiveNumThreads(3), 3);
}


int main(int argc, char** argv) {
    logging::initialize();

    test_thread_wait();
    test_thread_pause();
    test_thread_stop();
    test_thread_pause_stop();
    test_thread_restart();
    test_thread_valid_setup();
    test_thread_invalid_setup();
    test_thread_test_callback();
    test_thread_test_default_callback();
    test_thread_timer();
    test_thread_pool_no_arg_no_return();
    test_thread_pool_arg_no_return();
    test_thread_pool_no_arg_return();
    test_thread_pool_arg_return();
    test_thread_pool_destructor();
    test_thread_pool_stop();
    test_thread_pool_wait();
    test_thread_pool_wait_with_pause();
    test_thread_pool_wait_without_pause();
    test_thread_pool_wait_everytime();
    test_thread_pool_get_thread_index();
    test_thread_test_job_queue_single_producer_single_consumer();
    test_thread_test_job_queue_single_producer_single_consumer_max_num_jobs();
    test_thread_test_job_queue_multiple_producer_single_consumer();
    test_thread_test_job_queue_single_producer_multiple_consumer();
    test_thread_test_job_queue_multiple_producer_multiple_consumer();
    test_thread_test_job_queue_wait();
    test_thread_test_job_queue_stop_producer();
    test_thread_test_job_queue_stop_consumer();
    test_thread_test_job_queue_clear();
    test_thread_test_get_effective_num_threads();

    LOG(INFO) << "test succeeded";
    return 0;
}