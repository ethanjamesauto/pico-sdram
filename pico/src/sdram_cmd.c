#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "sdram_cmd.h"

#include "three_74hc595.pio.h"
#include "data_bus.pio.h"
#include "clkgen.pio.h"

#include <stdio.h>

// variable for storing all pio sm offsets, etc.
sdram_sm_t sdram_sm;

// switch bus mode between input and output
void switch_bus_mode(bool is_out) {
    if (is_out != sdram_sm.bus_mode) {
        sdram_sm.bus_mode = is_out;
        pio_sm_set_consecutive_pindirs(sdram_sm.pio2, sdram_sm.sm2, DATA_BASE, DATA_WIDTH, is_out);
        
        // set pullups
        // for (int i = 0; i < DATA_WIDTH; i++) gpio_set_pulls(DATA_BASE + i, !is_out, false);
    }
}

uint32_t get_addr_word(uint32_t a) {
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

void sm_resync() {
    // pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, false);

    // Reset both program counters to 0 by executing a 
    // JMP instruction to the starting offset of each program
    // pio_sm_exec(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset);
    // pio_sm_exec(sdram_sm.pio2, sdram_sm.sm2, sdram_sm.offset2);

    // empty rx fifo
    // TODO: better way to do this?
    //while (pio_sm_is_rx_fifo_empty(sdram_sm.pio2, sdram_sm.sm2) == false) {
    //    pio_sm_get_blocking(sdram_sm.pio2, sdram_sm.sm2);
    // }
    // pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, true);
}

void sm_resync_read() {
    pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, false);

    // Reset both program counters to 0 by executing a 
    // JMP instruction to the starting offset of each program
    pio_sm_exec(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset);
    pio_sm_exec(sdram_sm.pio2, sdram_sm.sm2, sdram_sm.offset2 + 3); // the read sm is at offset 3

    // empty rx fifo
    // TODO: better way to do this?
    while (pio_sm_is_rx_fifo_empty(sdram_sm.pio2, sdram_sm.sm2) == false) {
        pio_sm_get_blocking(sdram_sm.pio2, sdram_sm.sm2);
    }
}

void sm_start() {
    pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, true);
}

void test_pio() {

    switch_bus_mode(true); // set data bus to output mode

    for (int i = 0; i < 8; i++) {
        uint32_t dat = 0b000100000100000100000100;
        // uint32_t dat = 0b100000100000100000100000;
        // uint32_t dat = 0b100010001100010001100010;
        // uint32_t dat = 0b100100100100100100100100;
        dat = ((dat & 0b111111111111111111111000) << 5) | (dat & 0b111);


        uint32_t jmp_addr;
        if (i % 4 == 0) {
            jmp_addr = sdram_sm.offset + 7; // data

            for (int j = 0; j < 2; j++) {
                int num = j + (i > 0 ? 2 : 0);
                pio_sm_put_blocking(sdram_sm.pio2, sdram_sm.sm2, 2*num | ((2*num + 1) << 16));
            }
        } else {
            jmp_addr = sdram_sm.offset + 9;
        }
        dat |= (jmp_addr << 3);
        pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, dat << 3);
    }

    pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm, true);

    while (!pio_sm_is_tx_fifo_empty(sdram_sm.pio2, sdram_sm.sm2) || !pio_sm_is_tx_fifo_empty(sdram_sm.pio, sdram_sm.sm)) {
        tight_loop_contents();
    }

    // Note: this sleep is still necessary as the SMs could still be running even when the tx fifo is empty
    sleep_us(5);

    pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm, false);
    pio_sm_exec(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset | 0x1000);
}

void resync_pio() {
    pio_sm_exec(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset);
}

void sdram_init() {
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&three_74hc595_program, &sdram_sm.pio, &sdram_sm.sm, &sdram_sm.offset, CMD_SM_SIDESET_BASE, CMD_SM_TOTAL_PINS, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&data_bus_program, &sdram_sm.pio2, &sdram_sm.sm2, &sdram_sm.offset2, DATA_BASE, DATA_WIDTH, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&clkgen_program, &sdram_sm.pio3, &sdram_sm.sm3, &sdram_sm.offset3, SDRAM_CLK, 1, true);
    hard_assert(success);

    three_74hc595_program_init(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset, CMD_SM_SHIFT_OUT_BASE, CMD_SM_SIDESET_BASE);
    data_bus_program_init(sdram_sm.pio2, sdram_sm.sm2, sdram_sm.offset2, DATA_BASE);
    clkgen_program_init(sdram_sm.pio3, sdram_sm.sm3, sdram_sm.offset3, SDRAM_CLK);
    pio_clkdiv_restart_sm_mask(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2 | 1u << sdram_sm.sm3);

    sdram_sm.bus_mode = true; // default to output mode
    switch_bus_mode(false); // set data bus to input mode

    // set the CS pin to high impedance to make sure it doesn't short the shift register
    gpio_set_function(SDRAM_CS, GPIO_FUNC_NULL);
    gpio_disable_pulls(SDRAM_CS);
    gpio_set_input_enabled(SDRAM_CS, false); // make sure the input circuitry is disconnected - https://forums.raspberrypi.com/viewtopic.php?t=312533

}

void sdram_exec(uint32_t* cmd, uint16_t* data, uint32_t cmd_len, uint32_t data_len) {

    switch_bus_mode(true); // set data bus to output mode

    // sync the state machines
    sm_resync();

    for (int i = 0; i < data_len; i += 2) {
        if (i + 1 < cmd_len) {
            pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i]);
            pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i + 1]);
        }
        pio_sm_put_blocking(sdram_sm.pio2, sdram_sm.sm2, (data[i + 1] << 16) | data[i]);

        // Let the fifos fill up a bit before starting the pios
        // TODO: why can this go all the way up to 7 without failing? The fifos should be completely full and the program should be stuck
        if (i == 2) sm_start();
    }

    // wait for the last data to be sent
    sdram_wait();
}

// TODO: no need to store the data in the data array, just read it directly from the fifo
// same for sdram_exec
void sdram_exec_read(uint32_t* cmd, uint16_t* data, uint32_t cmd_len, uint32_t data_len) {

    switch_bus_mode(false); // set data bus to input mode

    // sync the state machines and clear the rx fifo
    sm_resync_read();

    int read_ptr = 0;

    for (int i = 0; i < data_len; i += 2) {
        if (i + 1 < cmd_len) {
            pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i]);
            pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i + 1]);
        }
        // pio_sm_put_blocking(sdram_sm.pio2, sdram_sm.sm2, (data[i + 1] << 16) | data[i]);

        // Let the fifos fill up a bit before starting the pios
        // TODO: why can this go all the way up to 7 without failing? The fifos should be completely full and the program should be stuck
        if (i == 2) sm_start();

        // if there's anything in the rx fifo, read it
        while (read_ptr < data_len && pio_sm_is_rx_fifo_empty(sdram_sm.pio2, sdram_sm.sm2) == false) {
            uint32_t d = pio_sm_get_blocking(sdram_sm.pio2, sdram_sm.sm2);
            data[read_ptr++] = d & 0xffff;
            data[read_ptr++] = d >> 16;
        }
    }

    // keep reading until we've read all the data
    while (read_ptr < data_len) {
        uint32_t d = pio_sm_get_blocking(sdram_sm.pio2, sdram_sm.sm2);
        data[read_ptr++] = d & 0xffff;
        data[read_ptr++] = d >> 16;
    }

    // wait for the last data to be sent
    // This should never be necessary, but it's here just in case
    sdram_wait();
}

/**
 * Wait for all operations to finish
 */
void sdram_wait() {
    while(pio_sm_is_tx_fifo_empty(sdram_sm.pio, sdram_sm.sm) == false || pio_sm_is_tx_fifo_empty(sdram_sm.pio2, sdram_sm.sm2) == false) {
        tight_loop_contents();
    }
    // this is needed to allow the final commands to be executed after the tx fifos are empty
    // TODO: find a more elegant solution
    sleep_us(1);
}

void sdram_write1(uint32_t addr, uint8_t bank, uint16_t data) {
    const int num_cmds = 4;
    const int num_data = 5;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    for (int i = 0; i < num_data; i++) dat[i] = 0;
    dat[3] = data;

    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9));
    cmd[1] = process_cmd(CMD_INHIBIT);

    // ADDR10 results in an auto-precharge
    cmd[2] = process_cmd(WRITE | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10); 
    cmd[3] = process_cmd(CMD_INHIBIT);

    sdram_exec(cmd, dat, num_cmds, num_data);
}

uint16_t sdram_read1(uint32_t addr, uint8_t bank) {
    const int num_cmds = 4;
    const int num_data = 8;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    for (int i = 0; i < num_data; i++) dat[i] = 0;
    
    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9));
    cmd[1] = process_cmd(CMD_INHIBIT);

    // ADDR10 results in an auto-precharge
    cmd[2] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10); 
    cmd[3] = process_cmd(CMD_INHIBIT);

    sdram_exec_read(cmd, dat, num_cmds, num_data);

    // print the entire data array in one line
    // for (int i = 0; i < num_data; i++) printf("%04x ", dat[i]);
    // printf("\n");

    return dat[3 + 3 + 1]; // with cas latency of 3, the data is 3 cycles after the read command
}

void sdram_read8(uint32_t addr, uint8_t bank, uint16_t* data) {
    const int num_cmds = 4;
    const int num_data = 16;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    for (int i = 0; i < num_data; i++) dat[i] = 0;
    
    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9));
    cmd[1] = process_cmd(CMD_INHIBIT);

    // ADDR10 results in an auto-precharge
    cmd[2] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10); 
    cmd[3] = process_cmd(CMD_INHIBIT);

    sdram_exec_read(cmd, dat, num_cmds, num_data);

    // print the entire data array in one line
    // for (int i = 0; i < num_data; i++) printf("%04d ", dat[i]);
    // printf("\n");

    for (int i = 0; i < 8; i++) {
        data[i] = dat[3 + 3 + 1 + i];
    }
}

void sdram_read4(uint32_t addr, uint8_t bank, uint16_t* data) {
    const int num_cmds = 4;
    const int num_data = 12;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    for (int i = 0; i < num_data; i++) dat[i] = 0;
    
    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9));
    cmd[1] = process_cmd(CMD_INHIBIT);

    // ADDR10 results in an auto-precharge
    cmd[2] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10); 
    cmd[3] = process_cmd(CMD_INHIBIT);

    sdram_exec_read(cmd, dat, num_cmds, num_data);

    // print the entire data array in one line
    // for (int i = 0; i < num_data; i++) printf("%04d ", dat[i]);
    // printf("\n");

    for (int i = 0; i < 4; i++) {
        data[i] = dat[3 + 3 + 1 + i];
    }
}

void sdram_read2(uint32_t addr, uint8_t bank, uint16_t* data) {
    const int num_cmds = 4;
    const int num_data = 10;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    for (int i = 0; i < num_data; i++) dat[i] = 0;
    
    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9));
    cmd[1] = process_cmd(CMD_INHIBIT);

    // ADDR10 results in an auto-precharge
    cmd[2] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10); 
    cmd[3] = process_cmd(CMD_INHIBIT);

    sdram_exec_read(cmd, dat, num_cmds, num_data);

    // print the entire data array in one line
    // for (int i = 0; i < num_data; i++) printf("%04d ", dat[i]);
    // printf("\n");

    for (int i = 0; i < 2; i++) {
        data[i] = dat[3 + 3 + 1 + i];
    }
}

// WIP - not finished yet
void sdram_read_page(uint32_t addr, uint8_t bank, uint16_t* data, uint8_t size) {
    const int num_cmds = 20;
    const int num_data = 30;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    for (int i = 0; i < num_data; i++) dat[i] = 0;
    
    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9));
    cmd[1] = process_cmd(CMD_INHIBIT);

    // ADDR10 results in an auto-precharge
    cmd[2] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10); 
    cmd[3] = process_cmd(CMD_INHIBIT);
    // cmd[16] = process_cmd(BURST_TERMINATE);

    sdram_exec_read(cmd, dat, num_cmds, num_data);

    // print the entire data array in one line
    for (int i = 0; i < num_data; i++) printf("%04d ", dat[i]);
    printf("\n");

    for (int i = 0; i < size; i++) {
        data[i] = dat[3 + 3 + 1 + i];
    }
}

void refresh_all() {
    // First, turn off the two state machines
    pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, false);

    // Reset both program counters to 0 by executing a 
    // JMP instruction to the starting offset of each program
    pio_sm_exec(sdram_sm.pio, sdram_sm.sm, sdram_sm.offset);
    // pio_sm_exec(sdram_sm.pio2, sdram_sm.sm2, sdram_sm.offset2);

    uint32_t cmd[5];
    cmd[0] = process_cmd(ACTIVATE);
    cmd[1] = process_cmd(CMD_INHIBIT);
    cmd[2] = process_cmd(PRECHARGE_ALL);
    cmd[3] = process_cmd(CMD_INHIBIT);
    cmd[4] = process_cmd(AUTO_REFRESH);

    for (int i = 0; i < 4; i++) {
        pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i]);
        if (i == 3) {
            pio_set_sm_mask_enabled(sdram_sm.pio, 1u << sdram_sm.sm | 1u << sdram_sm.sm2, true);
        }
    }
    
    for (int i = 0; i < 8192; i++) {
        pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[4]);
    }

    pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[1]);

    // wait for the refresh to finish
    sdram_wait();
}

void sdram_startup() {
    sm_resync();
    pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, process_cmd(CMD_INHIBIT | PIN_SDRAM_DQMH | PIN_SDRAM_DQML));
    sm_start();
    sleep_us(100);

    uint32_t cmd[6];
    cmd[0] = process_cmd(PRECHARGE_ALL);
    cmd[1] = process_cmd(AUTO_REFRESH);
    cmd[2] = process_cmd(AUTO_REFRESH);

    uint32_t mode = get_mode_word(MODE_BURST_LEN_1, MODE_ADDR_MODE_SEQUENTIAL, MODE_CAS_LATENCY_3, MODE_WRITE_MODE_SINGLE);
    cmd[3] = process_cmd(mode | LOAD_MODE);
    cmd[4] = process_cmd(CMD_INHIBIT);
    cmd[5] = process_cmd(AUTO_REFRESH);

    sm_resync();
    for (int i = 0; i < 6; i++) {
        pio_sm_put_blocking(sdram_sm.pio, sdram_sm.sm, cmd[i]);
        if (i == 2) sm_start();
    }

    // wait for the refresh to finish
    sdram_wait();
}