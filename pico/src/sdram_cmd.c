#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "sdram_cmd.h"

#include "three_74hc595.pio.h"
#include "data_bus.pio.h"
#include "clkgen.pio.h"
#include "hsync.pio.h"
#include "vsync.pio.h"

#include <stdio.h>

#define HSYNC_GPIO 28
#define VSYNC_GPIO 16

// variable for storing all pio sm offsets, etc.
sdram_sm_t sdram_sm;

// switch bus mode between input and output
void switch_bus_mode(bool is_out, uint32_t size) {
    if (is_out != sdram_sm.bus_mode) {
        // TODO: need to add checks to ensure all SMs have finished execution

        sdram_sm.bus_mode = is_out;
        pio_sm_set_consecutive_pindirs(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, DATA_BASE, DATA_WIDTH, is_out);
        
        pio_sm_exec(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, sdram_sm.data_bus_offset + (is_out ? 0 : 4));        
        // set pullups
        // for (int i = 0; i < DATA_WIDTH; i++) gpio_set_pulls(DATA_BASE + i, !is_out, false);

        // set pulldowns
        for (int i = 0; i < DATA_WIDTH; i++) gpio_set_pulls(DATA_BASE + i, false, !is_out);
    }

    if (size != sdram_sm.data_size) {
        pio_sm_put_blocking(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, size - 1);
        pio_sm_exec(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, 0x6040);
        pio_sm_exec(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, 0xa022);
        sdram_sm.data_size = size;
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

void test_pio() {
    sdram_startup();

    // for (int i = 0; i < 8; i++) {
    //     sdram_write1(i, 0, i + 3);
    // }
    uint16_t dat[8];
    for (int i = 0; i < 8; i++) dat[i] = 8 + i;
    sdram_write8(0, 0, dat);

    sdram_read8(0, 0, dat);
    for (int i = 0; i < 8; i++) {
        printf("%d ", dat[i]);
    }
    printf("\n");
    // for (int i = 0; i < 1000; i++) sdram_read1(0, 0);
    // printf("%d\n", dat);
}

void debug_print() {
    printf("cmd_bus: %d, data_bus: %d, clkgen: %d, hsync: %d, vsync: %d\n", 
        sdram_sm.cmd_bus_pio, sdram_sm.data_bus_pio, sdram_sm.clkgen_pio, sdram_sm.hsync_pio, sdram_sm.vsync_pio);
}

void resync_all() {
    pio_set_sm_mask_enabled(sdram_sm.cmd_bus_pio, 1u << sdram_sm.cmd_bus_sm | 1u << sdram_sm.hsync_sm, false);
    pio_sm_set_enabled(sdram_sm.vsync_pio, sdram_sm.vsync_sm, false);

    pio_clkdiv_restart_sm_mask(sdram_sm.cmd_bus_pio, 1u << sdram_sm.cmd_bus_sm | 1u << sdram_sm.hsync_sm);
    pio_clkdiv_restart_sm_mask(sdram_sm.vsync_pio, 1u << sdram_sm.vsync_sm);

    pio_sm_exec(sdram_sm.hsync_pio, sdram_sm.hsync_sm, sdram_sm.hsync_offset);
    pio_sm_exec(sdram_sm.vsync_pio, sdram_sm.vsync_sm, sdram_sm.vsync_offset);

    while(pio_sm_is_exec_stalled(sdram_sm.hsync_pio, sdram_sm.hsync_sm) || pio_sm_is_exec_stalled(sdram_sm.vsync_pio, sdram_sm.vsync_sm));
    sleep_ms(100);

    pio_set_sm_mask_enabled(sdram_sm.cmd_bus_pio, 1u << sdram_sm.cmd_bus_sm | 1u << sdram_sm.hsync_sm, true);
    pio_sm_set_enabled(sdram_sm.vsync_pio, sdram_sm.vsync_sm, true);
}

void vga_init() {
    for (int y = 0; y < 806; y++) {
        uint16_t page[512];
        for (int x = 0; x < 512; x++) {
            page[x] = 0;
            if (y < 768) {
                if (y < 64 || y > 768-64 || (y > 768/2 - 32 && y < 768/2 + 32))
                    page[x] = 0xffff;
                else if (x < 30 || x > 512-30)
                    page[x] = 0xffff;
            
            }
        }

        int addr = y * 512;
        sdram_write_page(addr, 0, page, 512);
    }
    sleep_ms(10);
}

void vga_send() {
    const size_t N = 168;
    uint32_t cmd[N];

    for (int i = 0; i < N; i++) cmd[i] = process_cmd(NOP, false);

    int bank = 0;
    int addr = 0;

    cmd[127] = process_cmd(BURST_TERMINATE, false); 
    cmd[128] = process_cmd(PRECHARGE | get_bank_word(bank), false); 

    pio_set_sm_mask_enabled(sdram_sm.cmd_bus_pio, 1u << sdram_sm.cmd_bus_sm | 1u << sdram_sm.hsync_sm, false);
    pio_sm_set_enabled(sdram_sm.vsync_pio, sdram_sm.vsync_sm, false);
    sleep_ms(1);

    bool first = true;
    while (1) {
        for (int i = 0; i < 806; i++) {
            addr = i * 512;
            cmd[N-2] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9), false);
            cmd[N-1] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff), false); 
            sdram_exec_cmd(cmd, N);
            if (first) {
                first = false;
                while(!pio_sm_is_tx_fifo_full(sdram_sm.cmd_bus_pio, sdram_sm.cmd_bus_sm));
                resync_all();
            }
        }
    }
}

void sdram_init() {
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&three_74hc595_program, &sdram_sm.cmd_bus_pio, &sdram_sm.cmd_bus_sm, &sdram_sm.cmd_bus_offset, CMD_SM_SIDESET_BASE, CMD_SM_TOTAL_PINS, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&data_bus_program, &sdram_sm.data_bus_pio, &sdram_sm.data_bus_sm, &sdram_sm.data_bus_offset, DATA_BASE, DATA_WIDTH, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&clkgen_program, &sdram_sm.clkgen_pio, &sdram_sm.clkgen_sm, &sdram_sm.clkgen_offset, SDRAM_CLK, 1, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&hsync_program, &sdram_sm.hsync_pio, &sdram_sm.hsync_sm, &sdram_sm.hsync_offset, HSYNC_GPIO, 1, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&vsync_program, &sdram_sm.vsync_pio, &sdram_sm.vsync_sm, &sdram_sm.vsync_offset, VSYNC_GPIO, 1, true);
    hard_assert(success);

    hard_assert(sdram_sm.clkgen_pio == sdram_sm.hsync_pio);
    hard_assert(sdram_sm.vsync_pio != sdram_sm.hsync_pio);

    three_74hc595_program_init(sdram_sm.cmd_bus_pio, sdram_sm.cmd_bus_sm, sdram_sm.cmd_bus_offset, CMD_SM_SHIFT_OUT_BASE, CMD_SM_SIDESET_BASE);
    data_bus_program_init(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, sdram_sm.data_bus_offset, DATA_BASE);
    clkgen_program_init(sdram_sm.clkgen_pio, sdram_sm.clkgen_sm, sdram_sm.clkgen_offset, SDRAM_CLK);

    hsync_program_init(sdram_sm.hsync_pio, sdram_sm.hsync_sm, sdram_sm.hsync_offset, HSYNC_GPIO);
    vsync_program_init(sdram_sm.vsync_pio, sdram_sm.vsync_sm, sdram_sm.vsync_offset, VSYNC_GPIO);

    // sync clock dividers and start clkgen/data bus sm
    // pio_set_sm_mask_enabled(sdram_sm.cmd_bus_pio, 1u << sdram_sm.cmd_bus_sm | 1u << sdram_sm.data_bus_sm | 1u << sdram_sm.clkgen_sm, false); // not needed - they should already be turned off
    pio_clkdiv_restart_sm_mask(sdram_sm.cmd_bus_pio, 1u << sdram_sm.cmd_bus_sm | 1u << sdram_sm.data_bus_sm | 1u << sdram_sm.clkgen_sm);
    pio_set_sm_mask_enabled(sdram_sm.cmd_bus_pio, 1u << sdram_sm.cmd_bus_sm | 1u << sdram_sm.data_bus_sm | 1u << sdram_sm.clkgen_sm, true);

    sdram_sm.bus_mode = true;
    sdram_sm.data_size = 0;
    switch_bus_mode(false, 8); // set data bus to input mode wih burst size of 8

    // set the CS pin to high impedance to make sure it doesn't short the shift register
    // gpio_set_function(SDRAM_CS, GPIO_FUNC_NULL);
    // gpio_disable_pulls(SDRAM_CS);
    // gpio_set_input_enabled(SDRAM_CS, false); // make sure the input circuitry is disconnected - https://forums.raspberrypi.com/viewtopic.php?t=312533

    sdram_sm.cmd_chan = dma_claim_unused_channel(true);
    sdram_sm.read_chan = dma_claim_unused_channel(true);
    sdram_sm.write_chan = dma_claim_unused_channel(true);

    dma_channel_config c;

    c = dma_channel_get_default_config(sdram_sm.read_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, false));

    dma_channel_configure(
        sdram_sm.read_chan,                         // Channel to be configured
        &c,                                         // The configuration we just created
        0,                                          // The initial write address (we set this later)
        &sdram_sm.data_bus_pio->rxf[sdram_sm.data_bus_sm],          // The initial read address
        0,                                          // Number of transfers; in this case each is 1 byte.
        false                                       // Don't start immediately.
    );

    c = dma_channel_get_default_config(sdram_sm.write_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(sdram_sm.data_bus_pio, sdram_sm.data_bus_sm, true));

    dma_channel_configure(
        sdram_sm.write_chan,                        // Channel to be configured
        &c,                                         // The configuration we just created
        &sdram_sm.data_bus_pio->txf[sdram_sm.data_bus_sm],          // The initial write address 
        0,                                          // The initial read address (we set this later)
        0,                                          // Number of transfers; in this case each is 1 byte.
        false                                       // Don't start immediately.
    );

    c = dma_channel_get_default_config(sdram_sm.cmd_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(sdram_sm.cmd_bus_pio, sdram_sm.cmd_bus_sm, true));

    dma_channel_configure(
        sdram_sm.cmd_chan,                          // Channel to be configured
        &c,                                         // The configuration we just created
        &sdram_sm.cmd_bus_pio->txf[sdram_sm.cmd_bus_sm],            // The initial write address
        0,                                          // The initial read address (we set this later)
        0,                                          // Number of transfers; in this case each is 1 byte.
        false                                       // Don't start immediately.
    );
}

void sdram_exec(uint32_t* cmd, uint16_t* data, uint32_t cmd_len, uint32_t data_len) {

    switch_bus_mode(true, data_len); // set data bus to output mode

    dma_channel_set_read_addr(sdram_sm.write_chan, data, false);
    dma_channel_set_trans_count(sdram_sm.write_chan, data_len/2, true);

    dma_channel_set_read_addr(sdram_sm.cmd_chan, cmd, false);
    dma_channel_set_trans_count(sdram_sm.cmd_chan, cmd_len, true);

    dma_channel_wait_for_finish_blocking(sdram_sm.cmd_chan);
    dma_channel_wait_for_finish_blocking(sdram_sm.write_chan);
}

// TODO: no need to store the data in the data array, just read it directly from the fifo
// same for sdram_exec
void sdram_exec_read(uint32_t* cmd, uint16_t* data, uint32_t cmd_len, uint32_t data_len) {

    switch_bus_mode(false, data_len); // set data bus to input mode

    dma_channel_set_write_addr(sdram_sm.read_chan, data, false);
    dma_channel_set_trans_count(sdram_sm.read_chan, data_len/2, true);

    dma_channel_set_read_addr(sdram_sm.cmd_chan, cmd, false);
    dma_channel_set_trans_count(sdram_sm.cmd_chan, cmd_len, true);

    dma_channel_wait_for_finish_blocking(sdram_sm.read_chan);
    dma_channel_wait_for_finish_blocking(sdram_sm.cmd_chan);
}

void sdram_exec_cmd(uint32_t* cmd, uint32_t cmd_len) {
    switch_bus_mode(false, 1); // set data bus to input mode

    dma_channel_wait_for_finish_blocking(sdram_sm.cmd_chan);
    dma_channel_set_read_addr(sdram_sm.cmd_chan, cmd, false);
    dma_channel_set_trans_count(sdram_sm.cmd_chan, cmd_len, true);
}

void sdram_write1(uint32_t addr, uint8_t bank, uint16_t data) {
    const int num_cmds = 4;
    const int num_data = 2;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    dat[0] = data;
    dat[1] = 0;

    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9), false);

    // ADDR10 results in an auto-precharge
    cmd[1] = process_cmd(WRITE | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10, true); 
    cmd[2] = process_cmd(NOP, false);
    cmd[3] = process_cmd(NOP, false);

    sdram_exec(cmd, dat, num_cmds, num_data);
}

uint16_t sdram_read1(uint32_t addr, uint8_t bank) {
    const int num_cmds = 4;
    const int num_data = 2;
    uint32_t cmd[num_cmds];
    uint16_t dat[num_data];

    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9), false);

    // ADDR10 results in an auto-precharge
    cmd[1] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10, true); 
    cmd[2] = process_cmd(NOP, false);
    cmd[3] = process_cmd(NOP, false);    
    
    sdram_exec_read(cmd, dat, num_cmds, num_data);

    return dat[0]; // with cas latency of 3, the data is 3 cycles after the read command
}

void sdram_read8(uint32_t addr, uint8_t bank, uint16_t* data) {
    const int num_cmds = 4;
    const int num_data = 8;
    uint32_t cmd[num_cmds];

    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9), false);

    // ADDR10 results in an auto-precharge
    cmd[1] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10, true); 
    cmd[2] = process_cmd(NOP, false); 
    cmd[3] = process_cmd(NOP, false); 
    
    sdram_exec_read(cmd, data, num_cmds, num_data);
}

void sdram_write8(uint32_t addr, uint8_t bank, uint16_t* data) {
    const int num_cmds = 4;
    const int num_data = 8;
    uint32_t cmd[num_cmds];

    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9), false);

    // ADDR10 results in an auto-precharge
    cmd[1] = process_cmd(WRITE | get_bank_word(bank) | get_addr_word(addr & 0x1ff) | PIN_SDRAM_ADDR10, true);
    cmd[2] = process_cmd(NOP, false);
    cmd[3] = process_cmd(NOP, false);
    
    sdram_exec(cmd, data, num_cmds, num_data);
}

void sdram_read_page(uint32_t addr, uint8_t bank, uint16_t* data, uint16_t num_data) {
    int burst_term = num_data / 8;
    int num_cmds = 2 + burst_term + 1 + 1;
    uint32_t cmd[num_cmds];

    for (int i = 0; i < num_cmds; i++) cmd[i] = process_cmd(NOP, false);

    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9), false);

    // ADDR10 results in an auto-precharge
    cmd[1] = process_cmd(READ | get_bank_word(bank) | get_addr_word(addr & 0x1ff), true); 

    cmd[1 + burst_term] = process_cmd(BURST_TERMINATE, false); 
    cmd[2 + burst_term] = process_cmd(PRECHARGE | get_bank_word(bank), false); 
    
    sdram_exec_read(cmd, data, num_cmds, num_data);
}

void sdram_write_page(uint32_t addr, uint8_t bank, uint16_t* data, uint16_t num_data) {
    int burst_term = num_data / 8;
    int num_cmds = 2 + burst_term + 1 + 1;
    uint32_t cmd[num_cmds];

    for (int i = 0; i < num_cmds; i++) cmd[i] = process_cmd(NOP, false);

    cmd[0] = process_cmd(ACTIVATE | get_bank_word(bank) | get_addr_word(addr >> 9), false);

    // ADDR10 results in an auto-precharge
    cmd[1] = process_cmd(WRITE | get_bank_word(bank) | get_addr_word(addr & 0x1ff), true); 

    cmd[1 + burst_term] = process_cmd(BURST_TERMINATE, false); 
    cmd[2 + burst_term] = process_cmd(PRECHARGE | get_bank_word(bank), false); 
    
    sdram_exec(cmd, data, num_cmds, num_data);
}

void refresh_all() {
    uint32_t cmd[8];
    for (int i = 0; i < 8; i++) cmd[i] = process_cmd(AUTO_REFRESH, false);
    for (int i = 0; i < 8192/8 + 1; i++) sdram_exec(cmd, 0, 8, 0);
}

void sdram_startup() {
    uint32_t cmd[7];
    cmd[0] = process_cmd(PRECHARGE_ALL, false);
    cmd[1] = process_cmd(AUTO_REFRESH, false);
    cmd[2] = process_cmd(AUTO_REFRESH, false);

    uint32_t mode = get_mode_word(MODE_BURST_LEN_FULL, MODE_ADDR_MODE_SEQUENTIAL, MODE_CAS_LATENCY_3, MODE_WRITE_MODE_BURST);
    cmd[3] = process_cmd(mode | LOAD_MODE, false);
    cmd[4] = process_cmd(NOP, false);
    cmd[5] = process_cmd(AUTO_REFRESH, false);
    cmd[6] = process_cmd(NOP, false);

    sdram_exec(cmd, 0, 7, 0);
}

inline uint32_t process_cmd(uint32_t cmd, bool is_rw) {
    cmd |= PIN_SDRAM_CKE;
    cmd = ((cmd & 0b111111111111111111111000) << 5) | (cmd & 0b111);
    
    uint32_t jmp_addr = sdram_sm.cmd_bus_offset + 10;

    if (is_rw) {
        jmp_addr = sdram_sm.cmd_bus_offset + 8;
    }

    cmd |= (jmp_addr << 3);
    return cmd << 3;
}