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

    while(1) {
        // sdram_exec(cmd, data, NUM_CMD, NUM_DATA);
        // refresh_all();
        //sdram_write1(0x3fffff, 0, 0x13);
        uint16_t data = sdram_read1(0x3fffff, 0);
        // sleep_ms(1);
        // sdram_startup();
        sleep_ms(50);
    }

    return 0;
}
