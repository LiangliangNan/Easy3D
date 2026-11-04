// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/util/timer.h>
#include <future>
#include <memory>
#include <chrono>
using namespace easy3d;
int main() { return test::run("bounded timer delivery and stop",[] {
    auto result=std::make_shared<std::promise<int>>();auto future=result->get_future();
    Timer<int>::single_shot(10,std::function<void(int)>([result](int value){result->set_value(value);}),42);
    test::require(future.wait_for(std::chrono::seconds(2))==std::future_status::ready && future.get()==42,"single-shot delivery timed out or changed arguments");
    // Stop on the timer's own thread; keep its owner alive until that thread exits.
    auto timer=std::make_shared<Timer<>>();auto done=std::make_shared<std::promise<void>>();auto ready=done->get_future();
    timer->set_timeout(10,std::function<void()>([timer,done]{timer->stop();done->set_value();}));
    test::require(ready.wait_for(std::chrono::seconds(2))==std::future_status::ready,"timeout did not fire");
    ready.get();test::require(timer->is_stopped(),"timer did not stop");
}); }
