#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "three_74hc595.pio.h"
#include "data_bus.pio.h"

#define SHIFT_OUT_BASE 19
#define SIDESET_BASE 17

#define DATA_BASE 0
#define DATA_WIDTH 16

#define TOTAL_PINS 5

int main()
{
    stdio_init_all();

    puts("Hello, world!");

    PIO pio;
    uint sm;
    uint offset;
    
    PIO pio2;
    uint sm2;
    uint offset2;


    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&three_74hc595_program, &pio, &sm, &offset, SIDESET_BASE, TOTAL_PINS, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&data_bus_program, &pio2, &sm2, &offset2, DATA_BASE, DATA_WIDTH, true);
    hard_assert(success);

    printf("Successfully loaded pio programs\n");

    three_74hc595_program_init(pio, sm, offset, SHIFT_OUT_BASE, SIDESET_BASE);
    data_bus_program_init(pio2, sm2, offset2, DATA_BASE);
    pio_clkdiv_restart_sm_mask(pio, 1u << sm | 1u << sm2);

    while(1) {
        for (int i = 0; i < 4; i++) {
            pio_sm_put_blocking(pio, sm, 0b010001100010001100010001);
            pio_sm_put_blocking(pio, sm, ~(0b010001100010001100010001));
            if (i != 3)
                pio_sm_put_blocking(pio2, sm2, 0x00020001);
            //pio_sm_put_blocking(pio2, sm2, 0x00080004);

            //pio_sm_put_blocking(pio2, sm2, 0xaaaa5555);
            //pio_sm_put_blocking(pio2, sm2, 0xaaaa5555);
        }

        sleep_ms(50);
    }

    return 0;
}
