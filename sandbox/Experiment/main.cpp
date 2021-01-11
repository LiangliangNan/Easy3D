#include <easy3d/core/signal.h>
#include <iostream>


using namespace easy3d;


class Car {
public:
    Car(int speed) : speed_(speed) {}
public:

    int speed() const { return speed_; }

    void start() {
        std::cout << "Car::" << __func__ << ". Class member: started\n";
    }

    void report_speed(int max_allow_speed) const {
        std::cout << "Car::" << __func__ << ". max allowed is " << max_allow_speed << ". I am at " << speed_ << "\n";
    }

    void stop(int hours, const std::string& msg) const {
        std::cout << "Car::" << __func__ << ". " << msg << ". After driving for " << hours << " hours\n";
    }

private:
    int speed_;
};


void func_start() {
    std::cout << __func__ << "\n";
}

void func_start_1arg(Car* car) {
    std::cout << __func__ << ". speed is " << car->speed() << "\n";
}

void func_report_speed(int max_allow_speed, const Car* car) {
    std::cout << __func__ << ". max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
}

void func_stop(const Car* car, int hours, const std::string& msg) {
    std::cout << __func__ << ". " << msg << ". After driving for " << hours << " hours, I have to stopped. My speed was " << car->speed() << "\n";
}


int main() {

    Car car(100);

    // connect to a class member ---------------------------------------------------------------------------------------

    Signal<>  start_signal;
    Signal<int>  report_signal;
    Signal<int, const std::string&>  stop_signal;

    // ---- connect to a class member, no argument
    easy3d::connect(&start_signal, &car, &Car::start);  // use the global function connect()
    start_signal.connect(&car, &Car::start);  // use signal's connect()

    // ---- connect to a const class member, one argument
    easy3d::connect(&report_signal, &car, &Car::report_speed);  // use the global function connect()
    report_signal.connect(&car, &Car::report_speed);  // use signal's connect()

    // ---- connect to a const class member, two arguments
    easy3d::connect(&stop_signal, &car, &Car::stop);  // use the global function connect()
    stop_signal.connect(&car, &Car::stop);  // use signal's connect()

    start_signal.send();
    report_signal.send(80);
    stop_signal.send(6, "I have to stop");

    // connect to a function -------------------------------------------------------------------------------------------

    Signal<> func_start_signal;
    Signal<Car*> func_start_signal_1arg;
    Signal<int, const Car*> func_report_signal;
    Signal<const Car*, int, const std::string&> another_stop_signal;

    // ---- connect to a function, no argument
    easy3d::connect(&func_start_signal, func_start);  // use the global function connect()
    func_start_signal.connect(func_start);  // use signal's connect()

    // ---- connect to a function, one argument

    easy3d::connect(&func_start_signal_1arg, func_start_1arg);  // use the global function connect()
    func_start_signal_1arg.connect(func_start_1arg);  // use signal's connect()

    // ---- connect to a function, two arguments
    easy3d::connect(&func_report_signal, func_report_speed);  // use the global function connect()
    func_report_signal.connect(func_report_speed);  // use signal's connect()

    // ---- connect to a function, three arguments
    easy3d::connect(&another_stop_signal, func_stop);  // use the global function connect()
    another_stop_signal.connect(func_stop);  // use signal's connect()

    func_start_signal.send();
    func_start_signal_1arg.send(&car);
    func_report_signal.send(80, &car);
    another_stop_signal.send(&car, 6, "I have to stop");


    // connect to a lambda function ------------------------------------------------------------------------------------

    auto lambda_start = []() -> void {
        std::cout << __func__ << "\n";
    };

    auto lambda_start_1arg = [](Car* car) -> void {
        std::cout << __func__ << ". speed is " << car->speed() << "\n";
    };

    auto lambda_report_speed = [](int max_allow_speed, const Car* car) -> void {
        std::cout << __func__ << ". max allowed is " << max_allow_speed << ". I am at " << car->speed() << "\n";
    };

    auto lambda_stop = [](const Car* car, int hours, const std::string& msg) -> void {
        std::cout << __func__ << ". " << msg << ". After driving for " << hours << " hours, I have to stopped. My speed was " << car->speed() << "\n";
    };

    Signal<> lambda_start_signal;
    Signal<Car*> lambda_start_signal_1arg;
    Signal<int, const Car*> lambda_report_signal;
    Signal<const Car*, int, const std::string&> lambda_stop_signal;

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
    lambda_start_signal_1arg.send(&car);
    lambda_report_signal.send(80, &car);
    lambda_stop_signal.send(&car, 6, "I have to stop");

    return 0;
}