#include <easy3d/util/timer.h>
#include <iostream>
#include <mutex>

std::mutex mutex;

using namespace easy3d;

// a simple class

class Car {
public:
    Car(int speed) : speed_(speed) {}

public:

    int speed() const { return speed_; }

    void start() {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "started\n";
    }

    void report_speed(int max_allow_speed) const {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "max allowed is " << max_allow_speed << ". I am at " << speed_ << "\n";
    }

    void stop(int hours, const std::string &msg) const {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << msg << ". After driving for " << hours << " hours\n";
    }

private:
    int speed_;
};


void test_for_members(Car* car) {
    // ---- a non-const class member, no argument
    Timer<>::single_shot(33, car, &Car::start);

    // ---- a const class member, one argument
    Timer<int>::single_shot(33, car, &Car::report_speed, 100);

    // ---- a const class member, two arguments
    Timer<int, const std::string&>::single_shot(33, car, &Car::stop, 6, "I have to stop");

    {   Timer<> t;
        t.single_shot(33, car, &Car::start);
        t.set_interval(33, car, &Car::start);
        t.set_timeout(33, car, &Car::start);
    }

    {   Timer<int> t;
        t.single_shot(33, car, &Car::report_speed, 100);
        t.set_interval(33, car, &Car::report_speed, 100);
        t.set_timeout(33, car, &Car::report_speed, 100);
    }

    {   Timer<int, const std::string&> t;
        t.single_shot(33, car, &Car::stop, 6, "I have to stop");
        t.set_interval(33, car, &Car::stop, 6, "I have to stop");
        t.set_timeout(33, car, &Car::stop, 6, "I have to stop");
    }
}


void func_start() {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << "started\n";
}

void func_start_1arg(Car* car) {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << "speed is " << car->speed() << "\n";
}

void func_report_speed(int max_allow_speed, const Car* car) {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << "max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
}

void func_stop(const Car* car, int hours, const std::string& msg) {
    std::lock_guard<std::mutex> guard(mutex);
    std::cout << msg << " after driving for " << hours << " hours. My speed was " << car->speed() << "\n";
}


void test_for_functions(Car* car) {
    // ---- no argument
    Timer<>::single_shot(33, func_start);

    // ---- one argument
    Timer<Car*>::single_shot(33, func_start_1arg, car);

    // ---- two argument
    Timer<int, const Car*>::single_shot(33, func_report_speed, 120, car);

    // ---- three arguments
    Timer<const Car*, int, const std::string&>::single_shot(333, func_stop, car,  6, "I have to stop");

    {   // ---- no argument
        Timer<> t;
        t.single_shot(33, func_start);
        t.set_interval(33, func_start);
        t.set_timeout(33, func_start);
    }

    {   // ---- one argument
        Timer<Car*> t;
        t.single_shot(33, func_start_1arg, car);
        t.set_interval(33, func_start_1arg, car);
        t.set_timeout(33, func_start_1arg, car);
    }

    {   // ---- two argument
        Timer<int, const Car*> t;
        t.single_shot(33, func_report_speed, 120, car);
        t.set_interval(33, func_report_speed, 120, car);
        t.set_timeout(33, func_report_speed, 120, car);
    }

    {   // ---- three arguments
        Timer<const Car*, int, const std::string&> t;
        t.single_shot(333, func_stop, car,  6, "I have to stop");
        t.set_interval(333, func_stop, car,  6, "I have to stop");
        t.set_timeout(333, func_stop, car,  6, "I have to stop");
    }
}


void test_for_lambda_functions(Car* car) {
    auto lambda_start = []() -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "started\n";
    };

    auto lambda_start_1arg = [](Car* car) -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "speed is " << car->speed() << "\n";
    };

    auto lambda_report_speed = [](int max_allow_speed, const Car* car) -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << "max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
    };

    auto lambda_stop = [](const Car* car, int hours, const std::string& msg) -> void {
        std::lock_guard<std::mutex> guard(mutex);
        std::cout << msg << " after driving for " << hours << " hours. My speed was " << car->speed() << "\n";
    };

    // ---- no argument
    Timer<>::single_shot(33, lambda_start);

    // ---- one argument
    Timer<Car*>::single_shot(33, lambda_start_1arg, car);

    // ---- two argument
    Timer<int, const Car*>::single_shot(33, lambda_report_speed, 120, car);

    // ---- three arguments
    Timer<const Car*, int, const std::string&>::single_shot(33, lambda_stop, car,  6, "I have to stop");

    {   // ---- no argument
        Timer<> t;
        t.single_shot(33, lambda_start);
        t.set_interval(33, lambda_start);
        t.set_timeout(33, lambda_start);
    }

    {   // ---- one argument
        Timer<Car*> t;
        t.single_shot(33, lambda_start_1arg, car);
        t.set_interval(33, lambda_start_1arg, car);
        t.set_timeout(33, lambda_start_1arg, car);
    }

    {   // ---- two argument
        Timer<int, const Car*> t;
        t.single_shot(33, lambda_report_speed, 120, car);
        t.set_interval(33, lambda_report_speed, 120, car);
        t.set_timeout(33, lambda_report_speed, 120, car);
    }

    {   // ---- three arguments
        Timer<const Car*, int, const std::string&> t;
        t.single_shot(333, lambda_stop, car,  6, "I have to stop");
        t.set_interval(333, lambda_stop, car,  6, "I have to stop");
        t.set_timeout(333, lambda_stop, car,  6, "I have to stop");
    }
}


int main() {
    Car car(100);

    std::cout << "triggers a class member ------------------------------------------------------------------\n\n";
    test_for_members(&car);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n\ntriggers a function ------------------------------------------------------------------\n\n";
    test_for_functions(&car);
    std::this_thread::sleep_for(std::chrono::seconds(1));


    std::cout << "\n\ntriggers a lambda function -----------------------------------------------------------\n\n";
    test_for_lambda_functions(&car);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return EXIT_SUCCESS;
}