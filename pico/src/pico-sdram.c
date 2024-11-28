#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "sdram_cmd.h"

int main()
{
    stdio_init_all();

    puts("Hello, world!");

    sdram_init();    

#define NUM_CMD 6
#define NUM_DATA 14
    uint32_t cmd[NUM_CMD];  // 16 32-bit words (only the lower 24 bits are used)
    uint16_t data[NUM_DATA]; // 64 16-bit words

    for (int i = 0; i < NUM_DATA; i++) {
        if (i >= 4 && i < 4+8) {// burst size - 2*4=8
            data[i] = 0xaaaa;
            if (i & 1) data[i] = ~data[i];
        } else {
            data[i] = 0;
        }
    }
    cmd[0] = process_cmd(ACTIVATE);
    cmd[1] = process_cmd(CMD_INHIBIT);
    cmd[2] = process_cmd(CMD_INHIBIT);

    uint32_t mode = get_mode_word(MODE_BURST_LEN_1, MODE_ADDR_MODE_SEQUENTIAL, MODE_CAS_LATENCY_3, MODE_WRITE_MODE_SINGLE);
    cmd[3] = process_cmd(mode | LOAD_MODE);

    cmd[4] = cmd_read(0b10000001, 0b11, true);
    cmd[5] = process_cmd(CMD_INHIBIT);

    while(1) {
        // sdram_exec(cmd, data, NUM_CMD, NUM_DATA);
        // refresh_all();
        // sleep_ms(1);
        sdram_startup();
        sleep_ms(50);
    }

    return 0;
}
