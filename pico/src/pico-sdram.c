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

void memtest_full_page(uint8_t bank) {
    static int succ = 0;
    static int fail = 0;

    const int max = 1 << 16; // just test bank 0

    const int num_write = 512;
    for (int i = 0; i < max; i += num_write) {
        uint16_t write_dat[num_write];

        for (int j = 0; j < num_write; j++) {
            write_dat[j] = get_data(i + j);
        }

        sdram_write_page(i, bank, write_dat, num_write);
        if (i % (1 << 15) == 0) refresh_all();
        if (i % (1 << 18) == 0) printf("Write Progress: %.1f%%\n", (float)i / (float)max * 100.0);
    }
    
    bool errors = false;

    const uint16_t num_read = 512;
    for (int i = 0; i < max; i += num_read) {
        uint16_t read_dat[num_read];
        sdram_read_page(i, bank, read_dat, num_read);

        if (i % (1 << 15) == 0) refresh_all();
        if (i % (1 << 18) == 0) printf("Read Progress: %.1f%%\n", (float)i / (float)max * 100.0);
        for (int j = 0; j < num_read; j++) {
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

int main()
{
    set_sys_clock_khz(120000, false);
    stdio_init_all();
    sdram_init();    
    sdram_startup();
    debug_init();

    vga_init();
    vga_send();
}
