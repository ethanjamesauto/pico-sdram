#include "debug.h"
#include "pico/stdlib.h"

void debug_init() {
    // init DEBUG_PIN gpio as output
    gpio_init(DEBUG_PIN);
    gpio_set_dir(DEBUG_PIN, GPIO_OUT);
    gpio_put(DEBUG_PIN, 0);
}

void debug_signal_error() {
    gpio_put(DEBUG_PIN, 1);
    sleep_us(1);
    gpio_put(DEBUG_PIN, 0);
}