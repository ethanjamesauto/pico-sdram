#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "three_74hc595.pio.h"

#define SHIFT_OUT_BASE 19
#define SIDESET_BASE 17

#define TOTAL_PINS 5

int main()
{
    stdio_init_all();

    puts("Hello, world!");

    PIO pio;
    uint sm;
    uint offset;

    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&three_74hc595_program, &pio, &sm, &offset, SIDESET_BASE, TOTAL_PINS, true);
    hard_assert(success);

    printf("Using gpio %d\n", SHIFT_OUT_BASE);
    three_74hc595_program_init(pio, sm, offset, SHIFT_OUT_BASE, SIDESET_BASE);
    while(1) {
        pio_sm_put_blocking(pio, sm, 0);
        pio_sm_put_blocking(pio, sm, 0b010001100010001100010001);
    }

    return 0;
}
