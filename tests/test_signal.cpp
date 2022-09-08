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


#include <easy3d/core/signal.h>
#include <iostream>
#include <string>


using namespace easy3d;


// A trivial class (that does not make sense, but just to show how to use the Signal class).

class MyCar {
public:
    MyCar(int speed) : speed_(speed) {}

public:
    int speed() const { return speed_; }

    void start() {
        std::cout << "started. speed: " << speed_ << "\n";
    }

    void start(const std::string& msg) {
        std::cout << "car started: " << msg << "\n";
    }

    void report_speed(int max_allow_speed) const {
        std::cout << "max allowed speed is " << max_allow_speed << ". I am at " << speed_ << "\n";
    }

    void stop(int hours, const std::string &msg) const {
        std::cout << msg << ". After driving for " << hours << " hours\n";
    }

private:
    int speed_;
};


void test_signal_for_members(MyCar *car) {
    Signal<> start_signal;
    Signal<const std::string&> start_signal_1arg;
    Signal<int> report_signal;
    Signal<int, const std::string &> stop_signal;

    // ---- connect to a class member, no argument

    start_signal.connect(car, &MyCar::start);  // this works
    start_signal.send();
    start_signal.connect(car, static_cast<void (MyCar::*)(void)> (&MyCar::start));  // this works
    start_signal.connect(car, overload<MyCar>(&MyCar::start));  // this works

//    easy3d::connect(&start_signal, car, &MyCar::start);  // this won't work
    easy3d::connect(&start_signal, car, static_cast<void (MyCar::*)(void)> (&MyCar::start));  // this works
    easy3d::connect(&start_signal, car, overload<MyCar>(&MyCar::start));  // this works

    start_signal_1arg.connect(car, &MyCar::start);  // this works
    start_signal_1arg.connect(car, static_cast<void (MyCar::*)(const std::string&)> (&MyCar::start));  // this works
    start_signal_1arg.connect(car, overload<MyCar, const std::string&>(&MyCar::start));  // this works

//    easy3d::connect(&start_signal_1arg, car, &MyCar::start);  // this won't work
    easy3d::connect(&start_signal_1arg, car, static_cast<void (MyCar::*)(const std::string&)> (&MyCar::start));  // this works
    easy3d::connect(&start_signal_1arg, car, overload<MyCar, const std::string&>(&MyCar::start));  // this works

    // ---- connect to a const class member, one argument

    easy3d::connect(&report_signal, car, &MyCar::report_speed);  // use the global function connect()
    report_signal.connect(car, &MyCar::report_speed);  // use signal's connect()

    // ---- connect to a const class member, two arguments

    easy3d::connect(&stop_signal, car, &MyCar::stop);  // use the global function connect()
    stop_signal.connect(car, &MyCar::stop);  // use signal's connect()

    // ---- emit all the signals

    start_signal.send();
    report_signal.send(80);
    stop_signal.send(6, "I have to stop");

    // ---- disconnect all signals

    start_signal.disconnect_all();
    report_signal.disconnect_all();
    stop_signal.disconnect_all();
}


void signal_func_start() {
    std::cout << "started\n";
}

void signal_func_start(const std::string& msg) {
    std::cout << "car started: " << msg << "\n";
}

void signal_func_report_speed(int max_allow_speed, const MyCar *car) {
    std::cout << "max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
}

void signal_func_stop(const MyCar *car, int hours, const std::string &msg) {
    std::cout << msg << " after driving for " << hours << " hours. My speed was " << car->speed() << "\n";
}


void test_signal_for_functions(MyCar *car) {
    Signal<> func_start_signal;
    Signal<const std::string &> func_start_signal_1arg;
    Signal<int, const MyCar *> func_report_signal;
    Signal<const MyCar *, int, const std::string &> another_stop_signal;

    // ---- connect to a function, no argument

//    func_start_signal.connect(signal_func_start);    // this won't work
    func_start_signal.connect(static_cast<void (*)(void)> (signal_func_start));    // this works
    func_start_signal.connect(overload<>(signal_func_start));    // this also works

//    easy3d::connect(&func_start_signal, signal_func_start);    // this won't work
    easy3d::connect(&func_start_signal, static_cast<void (*)(void)> (signal_func_start));  // this works
    easy3d::connect(&func_start_signal, overload<>(signal_func_start));    // this also works

    // ---- connect to a function, one argument

//    func_start_signal_1arg.connect(signal_func_start); // this won't work
    easy3d::connect(&func_start_signal_1arg, static_cast<void (*)(const std::string&)> (signal_func_start));   // this works
    easy3d::connect(&func_start_signal_1arg, overload<const std::string&>(signal_func_start));   // this works

//    func_start_signal_1arg.connect(signal_func_start);  // this won't work
    func_start_signal_1arg.connect(static_cast<void (*)(const std::string&)> (signal_func_start));   // this works
    func_start_signal_1arg.connect(overload<const std::string&> (signal_func_start));   // this works

    // ---- connect to a function, two arguments

    easy3d::connect(&func_report_signal, signal_func_report_speed);  // use the global function connect()
    func_report_signal.connect(signal_func_report_speed);  // use signal's connect()

    // ---- connect to a function, three arguments

    easy3d::connect(&another_stop_signal, signal_func_stop);  // use the global function connect()
    another_stop_signal.connect(signal_func_stop);  // use signal's connect()

    // ---- send all the signals

    func_start_signal.send();
    func_start_signal_1arg.send("blabla...");
    func_report_signal.send(80, car);
    another_stop_signal.send(car, 6, "I have to stop");

    func_start_signal.disconnect_all();
    func_start_signal_1arg.disconnect_all();
    func_report_signal.disconnect_all();
    another_stop_signal.disconnect_all();
}


void test_signal_for_lambda_functions(MyCar *car) {
    auto lambda_start = []() -> void {
        std::cout << "started\n";
    };

    auto lambda_start_1arg = [](MyCar *car) -> void {
        std::cout << "speed is " << car->speed() << "\n";
    };

    auto lambda_report_speed = [](int max_allow_speed, const MyCar *car) -> void {
        std::cout << "max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
    };

    auto lambda_stop = [](const MyCar *car, int hours, const std::string &msg) -> void {
        std::cout << msg << " after driving for " << hours << " hours. My speed was " << car->speed() << "\n";
    };

    Signal<> lambda_start_signal;
    Signal<MyCar *> lambda_start_signal_1arg;
    Signal<int, const MyCar *> lambda_report_signal;
    Signal<const MyCar *, int, const std::string &> lambda_stop_signal;

    // ---- connect to a function, no argument
    easy3d::connect(&lambda_start_signal, lambda_start);  // use the global function connect()
    lambda_start_signal.connect(lambda_start);  // use signal's connect()

    // ---- connect to a function, one argument

    easy3d::connect(&lambda_start_signal_1arg, lambda_start_1arg);  // use the global function connect()
    lambda_start_signal_1arg.connect(lambda_start_1arg);  // use signal's connect()

    // ---- connect to a function, two arguments
    easy3d::connect(&lambda_report_signal, lambda_report_speed);  // use the global function connect()
    lambda_report_signal.connect(lambda_report_speed);  // use signal's connect()

    // ---- connect to a function, three arguments
    easy3d::connect(&lambda_stop_signal, lambda_stop);  // use the global function connect()
    lambda_stop_signal.connect(lambda_stop);  // use signal's connect()

    lambda_start_signal.send();
    lambda_start_signal_1arg.send(car);
    lambda_report_signal.send(80, car);
    lambda_stop_signal.send(car, 6, "I have to stop");

    lambda_start_signal.disconnect_all();
    lambda_start_signal_1arg.disconnect_all();
    lambda_report_signal.disconnect_all();
    lambda_stop_signal.disconnect_all();
}


void test_signal_for_connect_signal_to_signal() {
    Signal<const std::string &> A;
    Signal<const std::string &> B;
    Signal<const std::string &> C;

    easy3d::connect(&A, &B);    // or A.connect(&B);
    easy3d::connect(&B, &C);    // or B.connect(&C);
    int id = C.connect([](const std::string &msg) -> void {
        std::cout << "C: " << msg << std::endl;
    });

    A.send("abc");

    A.disconnect(id);
}


int test_signal() {
    MyCar car(100);

    std::cout << "connect to a class member --------------------------------------------------------------\n";
    test_signal_for_members(&car);

    std::cout << "connect to a function ------------------------------------------------------------------\n";
    test_signal_for_functions(&car);

    std::cout << "connect to a lambda function -----------------------------------------------------------\n";
    test_signal_for_lambda_functions(&car);

    std::cout << "connect a signal to another signal -----------------------------------------------------\n";
    test_signal_for_connect_signal_to_signal();

    return EXIT_SUCCESS;
}
