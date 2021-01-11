#include <easy3d/core/signal.h>
#include <string>
#include <iostream>


using namespace easy3d;


class Car {
public:
    Car() {
        std::function<void(void)> stop1 = [this]() -> void { std::cout << "stopped\n" ; };
        auto stop2 = [this]() -> void { std::cout << "stopped\n" ; };

        stop_signal.connect(stop1);
        stop_signal.connect(stop2);
        easy3d::connect(&stop_signal, stop1);
        easy3d::connect(&stop_signal, stop2);
    }

    void stop() { stop_signal.send(); }


private:
    Signal<>  stop_signal;
};



int main() {
    Car car;
    car.stop();
    return 0;







    // create new signal
    Signal<std::string, int> signal;

    // attach a slot
    signal.connect([](std::string arg1, int arg2) {
        std::cout << arg1 << " " << arg2 << std::endl;
    });

    signal.send("The answer:", 42);

    return 0;
}