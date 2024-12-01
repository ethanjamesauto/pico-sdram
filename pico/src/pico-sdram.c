#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "sdram_cmd.h"

int main()
{
    stdio_init_all();
    sdram_init();    
    sdram_startup();

    while(1) {
        for (int i = 0; i < 10; i++) {
            sdram_write1(0x3fffff, 0, 0x13);
            // sleep_us(10);
            uint16_t data = sdram_read1(0x3fffff, 0); 
            sleep_us(1); // TODO: get rid of this
        }
        refresh_all();
    }

    return 0;
}
