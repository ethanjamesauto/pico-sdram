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
        // sdram_write1(i, 0, 0xaaaa);
        sdram_read1(0, 0); 
        //sleep_ms(500);
        //refresh_all();
    }

    return 0;
}
