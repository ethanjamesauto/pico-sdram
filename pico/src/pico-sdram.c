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
        sdram_startup();
        sdram_wait();
        sdram_write1(0x3fffff, 0, 0x13);
        sdram_wait();
        uint16_t data = sdram_read1(0x3fffff, 0);
        printf("Read data: %d\n", data);
        sleep_ms(50);
    }

    return 0;
}
