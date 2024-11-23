# pico-sdram

This project attempts to interface a Raspberry Pi Pico with a [16MB SDRAM IC](https://www.winbond.com/resource-files/w9825g6kh_a04.pdf).


## Future Work
Eventually, I'd like to use this SDRAM interface with a PIO-based QSPI interface. The QSPI interface may then appear as a QSPI PSRAM and can be connected to the rp2350's QSPI bus. The pico can then translate "psram" accesses to sdram accesses, which would **allow the pico to natively access sdram in the address space**. This might not be possible.