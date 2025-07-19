#include "gpio.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

GPIO::GPIO(int pin_number) : pin(pin_number) {
    if (pin < 0 || pin > 27) {
        throw std::invalid_argument("Invalid GPIO pin (0–27 only)");
    }
}

void GPIO::set_pin(int pin_number) {
    if (pin_number < 0 || pin_number > 27) {
        throw std::invalid_argument("Invalid GPIO pin (0–27 only)");
    }
    pin = pin_number;
}

void GPIO::on() const {
    write_to_driver("1");
}

void GPIO::off() const {
    write_to_driver("0");
}

void GPIO::toggle(int times, int delay_ms) const {
    for (int i = 0; i < times; ++i) {
        on();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        off();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
}

void GPIO::write_to_driver(const std::string& value) const {
    std::ofstream ofs(PROC_PATH);
    if (!ofs) {
        std::cerr << "Failed to open " << PROC_PATH << " for writing\n";
        return;
    }
    ofs << pin << "," << value;
    if (!ofs) {
        std::cerr << "Failed to write to " << PROC_PATH << "\n";
    }
}
