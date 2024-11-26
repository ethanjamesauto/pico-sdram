#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "three_74hc595.pio.h"
#include "data_bus.pio.h"
#include "clkgen.pio.h"

#include "sdram_cmd.h"

#define SHIFT_OUT_BASE 19
#define SIDESET_BASE 17

#define DATA_BASE 0
#define DATA_WIDTH 16

#define TOTAL_PINS 5

#define SDRAM_CLK 22

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

    PIO pio3;
    uint sm3;
    uint offset3;


    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&three_74hc595_program, &pio, &sm, &offset, SIDESET_BASE, TOTAL_PINS, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&data_bus_program, &pio2, &sm2, &offset2, DATA_BASE, DATA_WIDTH, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&clkgen_program, &pio3, &sm3, &offset3, SDRAM_CLK, 1, true);
    hard_assert(success);

    printf("Successfully loaded pio programs\n");

    three_74hc595_program_init(pio, sm, offset, SHIFT_OUT_BASE, SIDESET_BASE);
    data_bus_program_init(pio2, sm2, offset2, DATA_BASE);
    clkgen_program_init(pio3, sm3, offset3, SDRAM_CLK);
    pio_clkdiv_restart_sm_mask(pio, 1u << sm | 1u << sm2 | 1u << sm3);
    

#define NUM_CMD 6
#define NUM_DATA 7
    uint32_t cmd[NUM_CMD];  // 16 32-bit words (only the lower 24 bits are used)
    uint32_t data[NUM_DATA]; // 64 16-bit words
    
    for (int i = 0; i < NUM_CMD; i++) {
        cmd[i] = 0xaaaaaaaa;
        if (i % 2 == 0) {
            cmd[i] = ~cmd[i];
        }
    }

    for (int i = 0; i < NUM_DATA; i++) {
        if (i >= 2 && i < 2+4) // burst size - 2*4=8
            data[i] = 0x5555aaaa;
        else
            data[i] = 0;
    }
    cmd[0] = process_cmd(ACTIVATE);
    cmd[1] = process_cmd(CMD_INHIBIT);
    cmd[2] = process_cmd(CMD_INHIBIT);
    cmd[3] = cmd_read(0xffff, 1, false);
    cmd[4] = process_cmd(CMD_INHIBIT);
    cmd[5] = process_cmd(CMD_INHIBIT);

    while(1) {
        sleep_us(500);

        // Note: I'm 99% sure that these calls can't be sped up any further

        // First, turn off the two state machines
        pio_set_sm_mask_enabled(pio, 1u << sm | 1u << sm2, false);

        // Reset both program counters to 0 by executing a 
        // JMP instruction to the starting offset of each program
        pio_sm_exec(pio, sm, offset);
        pio_sm_exec(pio2, sm2, offset2);

        for (int i = 0; i < NUM_DATA; i++) {
            int j = i*2;
            if (j + 1 < NUM_CMD) {
                pio_sm_put_blocking(pio, sm, cmd[j]);
                pio_sm_put_blocking(pio, sm, cmd[j + 1]);
            }
            pio_sm_put_blocking(pio2, sm2, data[i]);

            // Let the fifos fill up a bit before starting the pios
            // TODO: why can this go all the way up to 7 without failing? The fifos should be completely full and the program should be stuck
            if (i == 3) {
                pio_set_sm_mask_enabled(pio, 1u << sm | 1u << sm2, true);
            }
        }
    }

    return 0;
}
