#include "gpio.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        GPIO led(19); // Use GPIO pin 19

        std::cout << "Turning LED ON...\n";
        led.on();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Turning LED OFF...\n";
        led.off();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Toggling LED 3 times with 300ms delay...\n";
        led.toggle(20, 1000);
    } catch (const std::exception& e) {
        std::cerr << "GPIO error: " << e.what() << '\n';
    }

    return 0;
}
