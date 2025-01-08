// #define CONFIG_DEBUG
#define CONFIG_REV1
// #define CONFIG_REV2

// On rev 2.0, the CS pin is also connected to this gpio.
// Right now, I'm using the CS pin on the shift register, so this pin should be set to high impedance.
#define SDRAM_CS 26


#ifdef CONFIG_DEBUG

// shift register 1
#define PIN_SDRAM_CS (1 << 0*3)
#define PIN_SDRAM_RAS (1 << 1*3)
#define PIN_SDRAM_CAS (1 << 2*3)
#define PIN_SDRAM_WE (1 << 3*3)
#define PIN_SDRAM_CKE (1 << 4*3)
#define PIN_SDRAM_DQML (1 << 5*3)
#define PIN_SDRAM_DQMH (1 << 6*3)
#define PIN_SDRAM_ADDR10 (1 << 7*3)

// shift register 2
#define PIN_SDRAM_ADDR8 (2 << 0*3)
#define PIN_SDRAM_ADDR9 (2 << 1*3)
// #define PIN_SDRAM_ADDR10 (2 << 2*3)
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

#elif defined(CONFIG_REV1)
// PCB Rev 1.0 pinout
// Note for 74hc595:
// A - 0
// B - 1
// C - 2
// D - 3
// E - 4
// F - 5
// G - 6
// H - 7

// shift register 3 (SER3)
#define PIN_SDRAM_DQML (4 << 0*3)
#define PIN_SDRAM_DQMH (4 << 1*3)
#define PIN_SDRAM_CKE (4 << 2*3)
#define PIN_SDRAM_CS (4 << 3*3)
#define PIN_SDRAM_RAS (4 << 4*3)
#define PIN_SDRAM_CAS (4 << 5*3)
#define PIN_SDRAM_WE (4 << 6*3)

// shift register 2 (SER2)
#define PIN_SDRAM_BA0 (1 << 0*3)

#define PIN_SDRAM_ADDR3 (1 << 2*3)
#define PIN_SDRAM_ADDR2 (1 << 3*3)
#define PIN_SDRAM_ADDR1 (1 << 4*3)
#define PIN_SDRAM_ADDR0 (1 << 5*3)
#define PIN_SDRAM_ADDR10 (1 << 6*3)
#define PIN_SDRAM_BA1 (1 << 7*3)

// shift register 1 (SER1)

#define PIN_SDRAM_ADDR4 (2 << 0*3)
#define PIN_SDRAM_ADDR12 (2 << 1*3)
#define PIN_SDRAM_ADDR11 (2 << 2*3)
#define PIN_SDRAM_ADDR9 (2 << 3*3)
#define PIN_SDRAM_ADDR8 (2 << 4*3)
#define PIN_SDRAM_ADDR7 (2 << 5*3)
#define PIN_SDRAM_ADDR6 (2 << 6*3)
#define PIN_SDRAM_ADDR5 (2 << 7*3)
// #define PIN_SDRAM_ADDR10 (2 << 2*3)



#define DATA_BASE 0
#define DATA_WIDTH 16

#define CMD_SM_TOTAL_PINS 5
#define CMD_SM_SHIFT_OUT_BASE 19
#define CMD_SM_SIDESET_BASE 18

// This value is also hardcoded in the PIO state machines
// TODO: make this not the case
#define SDRAM_CLK 22

#else

// PCB Rev 2.0 pinout
// Note for 74hc595:
// A - 0
// B - 1
// C - 2
// D - 3
// E - 4
// F - 5
// G - 6
// H - 7

// shift register 3 (SER3)
// #define NOT_USED (4 << 0*3)
#define PIN_SDRAM_DQML (4 << 1*3)
#define PIN_SDRAM_WE (4 << 2*3)
#define PIN_SDRAM_CAS (4 << 3*3)
#define PIN_SDRAM_RAS (4 << 4*3)
#define PIN_SDRAM_CKE (4 << 5*3)
#define PIN_SDRAM_DQMH (4 << 6*3)
// #define NOT_USED (4 << 7*3)

// shift register 2 (SER2)
#define PIN_SDRAM_BA0 (1 << 0*3)
#define PIN_SDRAM_CS (1 << 1*3)
#define PIN_SDRAM_ADDR3 (1 << 2*3)
#define PIN_SDRAM_ADDR2 (1 << 3*3)
#define PIN_SDRAM_ADDR1 (1 << 4*3)
#define PIN_SDRAM_ADDR0 (1 << 5*3)
#define PIN_SDRAM_ADDR10 (1 << 6*3)
#define PIN_SDRAM_BA1 (1 << 7*3)

// shift register 1 (SER1)

#define PIN_SDRAM_ADDR4 (2 << 0*3)
#define PIN_SDRAM_ADDR12 (2 << 1*3)
#define PIN_SDRAM_ADDR11 (2 << 2*3)
#define PIN_SDRAM_ADDR9 (2 << 3*3)
#define PIN_SDRAM_ADDR8 (2 << 4*3)
#define PIN_SDRAM_ADDR7 (2 << 5*3)
#define PIN_SDRAM_ADDR6 (2 << 6*3)
#define PIN_SDRAM_ADDR5 (2 << 7*3)


#define DATA_BASE 0
#define DATA_WIDTH 16

#define CMD_SM_TOTAL_PINS 5
#define CMD_SM_SHIFT_OUT_BASE 19
#define CMD_SM_SIDESET_BASE 18

// This value is also hardcoded in the PIO state machines
// TODO: make this not the case
#define SDRAM_CLK 22

#endif