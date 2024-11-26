#include "pinout.h"

#define CMD_INHIBIT (PIN_SDRAM_CS)
#define NOP (PIN_SDRAM_RAS | PIN_SDRAM_CAS | PIN_SDRAM_WE)
#define BURST_TERMINATE (PIN_SDRAM_RAS | PIN_SDRAM_CAS)
#define READ (PIN_SDRAM_RAS | PIN_SDRAM_WE)                 // for read and read with auto precharge
#define WRITE (PIN_SDRAM_RAS)                               // for write and write with auto precharge
#define ACTIVATE (PIN_SDRAM_CAS | PIN_SDRAM_WE)

// what a beauty
inline uint32_t get_addr_word(uint32_t a) {
    uint32_t word = 0;
    word |= (a & 1) ? PIN_SDRAM_ADDR0 : 0;
    word |= ((a >> 1) & 1) ? PIN_SDRAM_ADDR1 : 0;
    word |= ((a >> 2) & 1) ? PIN_SDRAM_ADDR2 : 0;
    word |= ((a >> 3) & 1) ? PIN_SDRAM_ADDR3 : 0;
    word |= ((a >> 4) & 1) ? PIN_SDRAM_ADDR4 : 0;
    word |= ((a >> 5) & 1) ? PIN_SDRAM_ADDR5 : 0;
    word |= ((a >> 6) & 1) ? PIN_SDRAM_ADDR6 : 0;
    word |= ((a >> 7) & 1) ? PIN_SDRAM_ADDR7 : 0;
    word |= ((a >> 8) & 1) ? PIN_SDRAM_ADDR8 : 0;
    word |= ((a >> 9) & 1) ? PIN_SDRAM_ADDR9 : 0;
    word |= ((a >> 10) & 1) ? PIN_SDRAM_ADDR10 : 0;
    word |= ((a >> 11) & 1) ? PIN_SDRAM_ADDR11 : 0;
    word |= ((a >> 12) & 1) ? PIN_SDRAM_ADDR12 : 0;
    return word;
}

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

