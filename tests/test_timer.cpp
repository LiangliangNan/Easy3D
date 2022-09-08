/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/


#include <easy3d/util/timer.h>
#include <easy3d/core/signal.h>

#include <iostream>
#include <string>
#include <mutex>

std::mutex mutex;

using namespace easy3d;

// Trivial classes (that do not make sense, but can show the use of the Signal class).

class Vehicle {
public:
    virtual void start() {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "do nothing\n";
    }
};

class Car : public Vehicle {
public:
    Car(int speed) : speed_(speed) {}

public:
    int speed() const { return speed_; }

    void start() override {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "started\n";
    }

    void start(int new_speed) {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "speed changed from " << speed_ << " to " << new_speed << "\n";
        speed_ = new_speed;
    }

    void stop(int hours, const std::string &msg) const {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << msg << ". After driving for " << hours << " hours\n";
    }

private:
    int speed_;
};


void test_timer_for_members(Car *car) {
    // ---- a non-const class member, no argument
    Timer<>::single_shot(33, car, &Car::start);

    // ---- a non-const overloaded class member, one argument
    Timer<int>::single_shot(33, car, &Car::start, 100);

    // ---- a const class member, two arguments
    Timer<int, const std::string &>::single_shot(33, car, &Car::stop, 6, "I have to stop");

    {
        static Timer<> t;
        t.single_shot(33, car, &Car::start);  // This works
        t.set_interval(33, car, &Car::start);          // This also works
        t.set_timeout(33, car, &Car::start);
        Timer<>::single_shot(500, &t, &Timer<>::stop);
    }

    {
        static Timer<int> t;
        t.single_shot(33, car, &Car::start, 100);
        t.set_interval(33, car, &Car::start, 100);
        t.set_timeout(33, car, &Car::start, 100);
        Timer<>::single_shot(500, &t, &Timer<int>::stop);
    }

    {
        static Timer<int, const std::string &> t;
        t.single_shot(33, car, &Car::stop, 6, "I have to stop");
        t.set_interval(33, car, &Car::stop, 6, "I have to stop");
        t.set_timeout(33, car, &Car::stop, 6, "I have to stop");
        Timer<>::single_shot(500, &t, &Timer<int, const std::string&>::stop);
    }
}


void timer_func_start() {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << "started\n";
}

void timer_func_start(Car *car) {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << "speed is " << car->speed() << "\n";
}

void timer_func_report_speed(int max_allow_speed, const Car *car) {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << "max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
}

void timer_func_stop(const Car *car, int hours, const std::string &msg) {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << msg << " after driving for " << hours << " hours. My speed was " << car->speed() << "\n";
}


void test_timer_for_functions(Car *car) {
    // ---- no argument

    Timer<>::single_shot(33, static_cast<void (*)(void)> (timer_func_start));
    Timer<>::single_shot(33, overload<>(timer_func_start));   // also works

    // ---- one argument
    Timer<Car *>::single_shot(33, static_cast<void (*)(Car *)> (timer_func_start), car);
    Timer<Car *>::single_shot(33, overload<Car *> (timer_func_start), car);


    // ---- two argument
    Timer<int, const Car *>::single_shot(33, timer_func_report_speed, 120, car);

    // ---- three arguments
    Timer<const Car *, int, const std::string &>::single_shot(333, timer_func_stop, car, 6, "I have to stop");

    {   // ---- no argument
        static Timer<> t;
        t.single_shot(33, static_cast<void (*)(void)> (timer_func_start));
        t.single_shot(33, overload<>(timer_func_start));   // also works

        t.set_interval(33, static_cast<void (*)(void)> (timer_func_start));
        t.set_interval(33, overload<>(timer_func_start));   // also works

        t.set_timeout(33, static_cast<void (*)(void)> (timer_func_start));
        t.set_timeout(33, overload<>(timer_func_start));   // also works

        Timer<>::single_shot(500, &t, &Timer<>::stop);
    }

    {   // ---- one argument
        static Timer<Car *> t;
        t.single_shot(33, static_cast<void (*)(Car *)> (timer_func_start), car);
        t.single_shot(33, overload<Car*> (timer_func_start), car);  // also works

        t.set_interval(33, static_cast<void (*)(Car *)> (timer_func_start), car);
        t.set_interval(33, overload<Car*>  (timer_func_start), car);  // also works

        t.set_timeout(33, static_cast<void (*)(Car *)> (timer_func_start), car);
        t.set_timeout(33, overload<Car*> (timer_func_start), car);  // also works

        Timer<>::single_shot(500, &t, &Timer<Car*>::stop);
    }

    {   // ---- two argument
        static Timer<int, const Car *> t;
        t.single_shot(33, timer_func_report_speed, 120, car);
        t.set_interval(33, timer_func_report_speed, 120, car);
        t.set_timeout(33, timer_func_report_speed, 120, car);
        Timer<>::single_shot(500, &t, &Timer<int, const Car*>::stop);
    }

    {   // ---- three arguments
        static Timer<const Car *, int, const std::string &> t;
        t.single_shot(333, timer_func_stop, car, 6, "I have to stop");
        t.set_interval(333, timer_func_stop, car, 6, "I have to stop");
        t.set_timeout(333, timer_func_stop, car, 6, "I have to stop");
        Timer<>::single_shot(500, &t, &Timer<const Car *, int, const std::string &>::stop);
    }
}


void test_timer_for_lambda_functions(Car *car) {
    auto lambda_start = []() -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "started\n";
    };

    auto lambda_start_1arg = [](Car *car) -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "speed is " << car->speed() << "\n";
    };

    auto lambda_report_speed = [](int max_allow_speed, const Car *car) -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
    };

    auto lambda_stop = [](const Car *car, int hours, const std::string &msg) -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << msg << " after driving for " << hours << " hours. My speed was " << car->speed() << "\n";
    };

    // ---- no argument
    Timer<>::single_shot(33, lambda_start);

    // ---- one argument
    Timer<Car *>::single_shot(33, lambda_start_1arg, car);

    // ---- two argument
    Timer<int, const Car *>::single_shot(33, lambda_report_speed, 120, car);

    // ---- three arguments
    Timer<const Car *, int, const std::string &>::single_shot(33, lambda_stop, car, 6, "I have to stop");

    {   // ---- no argument
        static Timer<> t;
        t.single_shot(33, lambda_start);
        t.set_interval(33, lambda_start);
        t.set_timeout(33, lambda_start);
        Timer<>::single_shot(500, &t, &Timer<>::stop);
    }

    {   // ---- one argument
        static Timer<Car *> t;
        t.single_shot(33, lambda_start_1arg, car);
        t.set_interval(33, lambda_start_1arg, car);
        t.set_timeout(33, lambda_start_1arg, car);
        Timer<>::single_shot(500, &t, &Timer<Car *>::stop);
    }

    {   // ---- two argument
        static Timer<int, const Car *> t;
        t.single_shot(33, lambda_report_speed, 120, car);
        t.set_interval(33, lambda_report_speed, 120, car);
        t.set_timeout(33, lambda_report_speed, 120, car);
        Timer<>::single_shot(500, &t, &Timer<int, const Car *>::stop);
    }

    {   // ---- three arguments
        static Timer<const Car *, int, const std::string &> t;
        t.single_shot(333, lambda_stop, car, 6, "I have to stop");
        t.set_interval(333, lambda_stop, car, 6, "I have to stop");
        t.set_timeout(333, lambda_stop, car, 6, "I have to stop");
        Timer<>::single_shot(500, &t, &Timer<const Car *, int, const std::string &>::stop);
    }
}


int test_timer() {
    Car car(100);

    std::cout << "triggers a class member ------------------------------------------------------------------\n";
    test_timer_for_members(&car);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\ntriggers a function ------------------------------------------------------------------\n";
    test_timer_for_functions(&car);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\ntriggers a lambda function -----------------------------------------------------------\n";
    test_timer_for_lambda_functions(&car);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
