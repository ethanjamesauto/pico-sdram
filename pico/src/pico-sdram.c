#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include <stdlib.h>

#include "sdram_cmd.h"

// function used to fill all addresses with some data.
// uses only the first 8 bits of data, but
// the data should be different for each address
// use a hash function to generate the data
uint16_t get_data(uint32_t addr) {
    // return ((addr >> 16)*5 + (addr >> 8)*3 + addr) & 0b0000001111111111;
    return rand() & 0b0000001111111111;
}

void memtest() {
    const int max = 1 << 24; // this tests the full 256Mbits of the SDRAM

    srand(0);
    for (int i = 0; i < max; i++) {
        uint16_t write_dat = get_data(i);
        sdram_write1(i >> 2, i & 0b11, write_dat);
        if (i % 10000 == 0) refresh_all();
        if (i % 100000 == 0) printf("Write Progress: %.1f%%\n", (float)i / (float)max * 100.0);
    }
    

    srand(0);
    bool errors = false;

    for (int i = 0; i < max; i++) {
        uint16_t read_dat = sdram_read1(i >> 2, i & 0b11);

        if (i % 10000 == 0) refresh_all();
        if (i % 100000 == 0) printf("Read Progress: %.1f%%\n", (float)i / (float)max * 100.0);
        if (read_dat != get_data(i)) {
            printf("Read %d: %d", i, read_dat);
            printf(" (err: %016b)\n", read_dat ^ get_data(i));
            errors = true;
        }
    }
    if (!errors) printf("SUCCESS: Test completed with no errors!\n");
    else printf("ERROR: Errors found!\n");
}

int main()
{
    stdio_init_all();
    sdram_init();    
    sdram_startup();

    while(1) {
        memtest();
    }
}
