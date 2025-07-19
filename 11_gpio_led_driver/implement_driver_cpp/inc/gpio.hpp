#ifndef GPIO_HPP
#define GPIO_HPP

#include <string>

class GPIO {
private:
    const std::string PROC_PATH = "/proc/gpio_led_driver";
    int pin;

public:
    explicit GPIO(int pin_number);

    void set_pin(int pin_number);
    void on() const;
    void off() const;
    void toggle(int times, int delay_ms = 500) const;

private:
    void write_to_driver(const std::string& value) const;
};

#endif // GPIO_HPP
