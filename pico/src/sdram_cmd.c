#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "sdram_cmd.h"

#include "three_74hc595.pio.h"
#include "data_bus.pio.h"
#include "clkgen.pio.h"

// variable for storing all pio sm offsets, etc.
sdram_sm_t sdram_sm;

void sdram_init() {
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&three_74hc595_program, &sdram_sm.pio, &sdram_sm.sm, &sdram_sm.offset, SIDESET_BASE, TOTAL_PINS, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&data_bus_program, &sdram_sm.pio2, &sdram_sm.sm2, &sdram_sm.offset2, DATA_BASE, DATA_WIDTH, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&clkgen_program, &sdram_sm.pio3, &sdram_sm.sm3, &sdram_sm.offset3, SDRAM_CLK, 1, true);
    hard_assert(success);

    three_74hc595_program_init(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset, SHIFT_OUT_BASE, SIDESET_BASE);
    data_bus_program_init(sdram_sm.pio2, sdram_sm.sm2, sdram_sm.offset2, DATA_BASE);
    clkgen_program_init(sdram_sm.pio3, sdram_sm.sm3, sdram_sm.offset3, SDRAM_CLK);
    pio_clkdiv_restart_sm_mask(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2 | 1u << sdram_sm.sm3);
}

void sdram_exec(uint32_t* cmd, uint16_t* data, uint32_t cmd_len, uint32_t data_len) {
    // Note: I'm 99% sure that these calls can't be sped up any further
    // First, turn off the two state machines
    pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, false);

    // Reset both program counters to 0 by executing a 
    // JMP instruction to the starting offset of each program
    pio_sm_exec(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset);
    pio_sm_exec(sdram_sm.pio2, sdram_sm.sm2, sdram_sm.offset2);

    for (int i = 0; i < data_len; i += 2) {
        if (i + 1 < cmd_len) {
            pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i]);
            pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i + 1]);
        }
        pio_sm_put_blocking(sdram_sm.pio2, sdram_sm.sm2, (data[i + 1] << 16) | data[i]);

        // Let the fifos fill up a bit before starting the pios
        // TODO: why can this go all the way up to 7 without failing? The fifos should be completely full and the program should be stuck
        if (i == 2) {
            pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, true);
        }
    }
}

void refresh_all() {
    // First, turn off the two state machines
    pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, false);

    // Reset both program counters to 0 by executing a 
    // JMP instruction to the starting offset of each program
    pio_sm_exec(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset);
    // pio_sm_exec(sdram_sm.pio2, sdram_sm.sm2, sdram_sm.offset2);

    uint32_t cmd[5];
    cmd[0] = process_cmd(ACTIVATE);
    cmd[1] = process_cmd(CMD_INHIBIT);
    cmd[2] = process_cmd(PRECHARGE_ALL);
    cmd[3] = process_cmd(CMD_INHIBIT);
    cmd[4] = process_cmd(AUTO_REFRESH);

    for (int i = 0; i < 4; i++) {
        pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i]);
        if (i == 3) {
            pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, true);
        }
    }
    
    for (int i = 0; i < 8192; i++) {
        pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[4]);
    }

    pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[1]);
}