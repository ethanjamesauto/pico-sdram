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
    
    for (int i = 0; i < NUM_CMD; i++) {
        cmd[i] = 0xaaaaaaaa;
        if (i % 2 == 0) {
            cmd[i] = ~cmd[i];
        }
    }

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
    cmd[3] = cmd_read(0xffff, 1, false);
    cmd[4] = process_cmd(CMD_INHIBIT);
    cmd[5] = process_cmd(CMD_INHIBIT);

    while(1) {
        sleep_ms(50);
        // sdram_exec(cmd, data, NUM_CMD, NUM_DATA);
        refresh_all();
    }

    return 0;
}
