# pico-sdram

This project attempts to interface a Raspberry Pi Pico with a [32MB SDRAM IC](https://www.winbond.com/resource-files/w9825g6kh_a04.pdf).

<p>
  <img src="images/p0.png" width="200" />
  <img src="images/p1.png" width="200" /> 
  <img src="images/p2.png" width="200" />
  <img src="images/p3.png" width="200" />
</p>

# TODO:
* code the data read state machine
* add in the pin mappings from pcb
* find a way to send the same commands multiple times without using more CPU or memory (coding a better pio state machine)

## Future Work
Eventually, I'd like to use this SDRAM interface with a PIO-based QSPI interface. The QSPI interface may then appear as a QSPI PSRAM and can be connected to the rp2350's QSPI bus. The pico can then translate "psram" accesses to sdram accesses, which would **allow the pico to natively access sdram in the address space**. This might not be possible.

## Digilent Waveforms SDRAM Protocol Decoder
```js
function Value2Text(flag, value) {
  value = value & 0xf;
  if (value & 1) return "-";
  if (value == 14) return "NOP";
  if (value == 6) return "BURST_TERM";
  if (value == 10) return "READ";
  if (value == 2) return "WRITE";
  if (value == 12) return "ACTIVATE";
  if (value == 4) return "PRECHARGE";
  if (value == 8) return "AUTO_REFRESH";
  if (value == 0) return "LOAD_MODE";
  return "X";
}
Value2Text(1,1)
```