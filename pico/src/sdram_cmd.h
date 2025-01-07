#ifndef GRANDPARENT_H
#define GRANDPARENT_H

#include "pinout.h"

#define CMD_INHIBIT (PIN_SDRAM_CS)
#define NOP (PIN_SDRAM_RAS | PIN_SDRAM_CAS | PIN_SDRAM_WE)
#define BURST_TERMINATE (PIN_SDRAM_RAS | PIN_SDRAM_CAS)
#define READ (PIN_SDRAM_RAS | PIN_SDRAM_WE)                 // for read and read with auto precharge
#define WRITE (PIN_SDRAM_RAS)                               // for write and write with auto precharge
#define ACTIVATE (PIN_SDRAM_CAS | PIN_SDRAM_WE)
#define PRECHARGE (PIN_SDRAM_CAS)
#define PRECHARGE_ALL (PIN_SDRAM_CAS | PIN_SDRAM_ADDR10)
#define AUTO_REFRESH (PIN_SDRAM_WE)
#define LOAD_MODE 0

// bit positions in the sdram command register
#define MODE_BURST_LEN 0
#define MODE_ADDR_MODE 3
#define MODE_CAS_LATENCY 4
#define MODE_WRITE_MODE 9

#define MODE_BURST_LEN_1 0b000
#define MODE_BURST_LEN_2 0b001
#define MODE_BURST_LEN_4 0b010
#define MODE_BURST_LEN_8 0b011
#define MODE_BURST_LEN_FULL 0b111

#define MODE_ADDR_MODE_SEQUENTIAL 0
#define MODE_ADDR_MODE_INTERLEAVED 1

#define MODE_CAS_LATENCY_2 0b010
#define MODE_CAS_LATENCY_3 0b011

#define MODE_WRITE_MODE_BURST 0
#define MODE_WRITE_MODE_SINGLE 1

typedef struct {
    // cmd/addr bus sm
    PIO pio;
    uint sm;
    uint offset;
    
    // data bus sm
    PIO pio2;
    uint sm2;
    uint offset2;

    // clock generator sm
    PIO pio3;
    uint sm3;
    uint offset3;

    // true - output, false - input
    bool bus_mode;
} sdram_sm_t;

/**
 * Initialize the SDRAM state machines
 */
void sdram_init();

/**
 * Execute a sequence of commands and data
 */
void sdram_exec(uint32_t* cmd, uint16_t* data, uint32_t cmd_len, uint32_t data_len);

/**
 * Run an auto refresh command on all banks
 * Calling this every 60-ish ms is probably a good idea
 */
void refresh_all();

/**
 * Run the power-up initialization sequence
 */
void sdram_startup();

/**
 * Wait for the current command to finish
 */
void sdram_wait();

/**
 * Write a single 16-bit word to the SDRAM
 * The mode register MUST be configured correctly before calling this function
 * addr - 22-bit address
 * addr[8:0] - column address
 * addr[21:9] - row address
 * bank[1:0] - bank address
 */
void sdram_write1(uint32_t addr, uint8_t bank, uint16_t data);

/**
 * Read a single 16-bit word from the SDRAM
 * addr - 22-bit address
 * addr[8:0] - column address
 * addr[21:9] - row address
 * bank[1:0] - bank address
 */
uint16_t sdram_read1(uint32_t addr, uint8_t bank);

// These are burst read functions - they are not completely implemented yet
// as they do not update the mode register to configure the burst length
void sdram_read8(uint32_t addr, uint8_t bank, uint16_t* data);
void sdram_read4(uint32_t addr, uint8_t bank, uint16_t* data);
void sdram_read2(uint32_t addr, uint8_t bank, uint16_t* data);

/**
 * Perform a full page read.
 * Note that this function will simply read the given number of words at the given
 * address - it does NOT pay attention to page boundaries.
 * size - number of words to read from RAM.
 */
void sdram_read_page(uint32_t addr, uint8_t bank, uint16_t* data, uint8_t size);

/**
 * Change the direction of the data bus
 * is_out - true to set data bus to output mode, false to set to input mode
 */
void switch_bus_mode(bool is_out);

/**
 * Transform the memory address to a 32-bit word sent to 
 * the shift registers with the correct pinout
 */
uint32_t get_addr_word(uint32_t a);

inline uint32_t get_bank_word(uint32_t b) {
    uint32_t word = 0;
    word |= (b & 1) ? PIN_SDRAM_BA0 : 0;
    word |= ((b >> 1) & 1) ? PIN_SDRAM_BA1 : 0;
    return word;
}

inline uint32_t process_cmd(uint32_t cmd) {
    cmd |= PIN_SDRAM_CKE;
    return cmd << 8;
}

/**
 * Get a 32-bit word with the mode register settings
 * Returns a word that can be ORed with LOAD_MODE to load the mode register
 */
inline uint32_t get_mode_word(uint8_t burst_len, uint8_t addr_mode, uint8_t cas_latency, uint8_t write_mode) {
    uint32_t word = 0;
    word |= (burst_len & 0b111) << MODE_BURST_LEN;
    word |= (addr_mode & 1) << MODE_ADDR_MODE;
    word |= (cas_latency & 0b111) << MODE_CAS_LATENCY;
    word |= (write_mode & 1) << MODE_WRITE_MODE;
    return get_addr_word(word);
}

// Read: read a burst of data from the currently active row
// Read with auto precharge: as above, and precharge (close row) when done
// addr - column address
// bank - bank address
inline uint32_t cmd_read(uint16_t addr, uint8_t bank, bool precharge) {
    if (precharge) {
        addr |= 1 << 10; // set A10
    } else {
        addr &= ~(1 << 10); // clear A10
    }

    return process_cmd(READ | get_addr_word(addr) | get_bank_word(bank));
}

// Write: write a burst of data to the currently active row
// Write with auto precharge: as above, and precharge (close row) when done
// addr - column address
// bank - bank address
inline uint32_t cmd_write(uint16_t addr, uint8_t bank, bool precharge) {
    if (precharge) {
        addr |= 1 << 10; // set A10
    } else {
        addr &= ~(1 << 10); // clear A10
    }

    return process_cmd(WRITE | get_addr_word(addr) | get_bank_word(bank));
}

void test_pio();
void sm_resync();

#endif