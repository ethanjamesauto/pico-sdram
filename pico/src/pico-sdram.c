#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#include <stdlib.h>

#include "sdram_cmd.h"
#include "debug.h"

// function used to fill all addresses with some data.
// uses only the first 8 bits of data, but
// the data should be different for each address
// use a hash function to generate the data
uint16_t get_data(uint32_t addr) {
    srand(addr);
    return rand();
    // return ((addr >> 16)*5 + (addr >> 8)*3 + addr);
    //return rand();
}

void memtest() {
    const int max = 1 << 24; // this tests the full 256Mbits of the SDRAM

    for (int i = 0; i < max; i++) {
        uint16_t write_dat = get_data(i);
        sdram_write1(i >> 2, i & 0b11, write_dat);
        if (i % 10000 == 0) refresh_all();
        if (i % 100000 == 0) printf("Write Progress: %.1f%%\n", (float)i / (float)max * 100.0);
    }
    

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

void memtest_burst_8(uint8_t bank) {
    static int succ = 0;
    static int fail = 0;

    const int max = 1 << 22; // just test bank 0

    for (int i = 0; i < max; i += 8) {
        uint16_t write_dat[8];

        for (int j = 0; j < 8; j++) {
            write_dat[j] = get_data(i + j);
        }

        sdram_write8(i, bank, write_dat);
        if (i % 10000 == 0) refresh_all();
        if (i % 100000 == 0) printf("Write Progress: %.1f%%\n", (float)i / (float)max * 100.0);
    }
    

    bool errors = false;

    for (int i = 0; i < max; i += 16) {
        uint16_t read_dat[16];
        sdram_read16(i, bank, read_dat);

        if (i % 10000 == 0) refresh_all();
        if (i % 100000 == 0) printf("Read Progress: %.1f%%\n", (float)i / (float)max * 100.0);
        for (int j = 0; j < 16; j++) {
            if (read_dat[j] != get_data(i + j)) {
                printf("Read %d: %d", i + j, read_dat[j]);
                printf(" (err: %016b)\n", read_dat[j] ^ get_data(i + j));
                errors = true;
            }
        }
    }
    if (!errors) {
        printf("SUCCESS: Test completed! Successes - %d, Failures - %d\n", ++succ, fail);
    }
    else {
        printf("ERROR: Errors found! Successes - %d, Failures - %d\n", succ, ++fail);
    }
}

void memtest_full_page() {
    sdram_startup();
    for (int i = 0; i < 1024; i++) {
        sdram_write1(i, 0, i);
    }
    refresh_all();

    uint16_t data[2];
    sdram_read_page(32, 0, data, 2);
}

int main()
{
    set_sys_clock_khz(100000, false);
    stdio_init_all();
    sdram_init();    
    sdram_startup();
    debug_init();

    while(1) {
        // memtest_full_page();
        // test_pio();
        for (int i = 0; i < 4; i++) memtest_burst_8(i);
        // memtest();
    }
}
