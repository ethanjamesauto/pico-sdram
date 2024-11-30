
// shift register 1
#define PIN_SDRAM_CS (1 << 0*3)
#define PIN_SDRAM_RAS (1 << 1*3)
#define PIN_SDRAM_CAS (1 << 2*3)
#define PIN_SDRAM_WE (1 << 3*3)
#define PIN_SDRAM_CKE (1 << 4*3)
#define PIN_SDRAM_DQML (1 << 5*3)
#define PIN_SDRAM_DQMH (1 << 6*3)

// shift register 2
#define PIN_SDRAM_ADDR8 (2 << 0*3)
#define PIN_SDRAM_ADDR9 (2 << 1*3)
#define PIN_SDRAM_ADDR10 (2 << 2*3)
#define PIN_SDRAM_ADDR11 (2 << 3*3)
#define PIN_SDRAM_ADDR12 (2 << 4*3)
#define PIN_SDRAM_BA0 (2 << 5*3)
#define PIN_SDRAM_BA1 (2 << 6*3)

// shift register 3
#define PIN_SDRAM_ADDR0 (4 << 0*3)
#define PIN_SDRAM_ADDR1 (4 << 1*3)
#define PIN_SDRAM_ADDR2 (4 << 2*3)
#define PIN_SDRAM_ADDR3 (4 << 3*3)
#define PIN_SDRAM_ADDR4 (4 << 4*3)
#define PIN_SDRAM_ADDR5 (4 << 5*3)
#define PIN_SDRAM_ADDR6 (4 << 6*3)
#define PIN_SDRAM_ADDR7 (4 << 7*3)



#define DATA_BASE 0
#define DATA_WIDTH 16

#define CMD_SM_TOTAL_PINS 5
#define CMD_SM_SHIFT_OUT_BASE 19
#define CMD_SM_SIDESET_BASE 17

// This value is also hardcoded in the PIO state machines
// TODO: make this not the case
#define SDRAM_CLK 22