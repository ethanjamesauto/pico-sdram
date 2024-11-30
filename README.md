# pico-sdram

This project attempts to interface a Raspberry Pi Pico with a [32MB SDRAM IC](https://www.winbond.com/resource-files/w9825g6kh_a04.pdf).

<p>
  <img src="images/p0.png" width="200" />
  <img src="images/p1.png" width="200" /> 
  <img src="images/p2.png" width="200" />
  <img src="images/p3.png" width="200" />
</p>

## Future Work
Eventually, I'd like to use this SDRAM interface with a PIO-based QSPI interface. The QSPI interface may then appear as a QSPI PSRAM and can be connected to the rp2350's QSPI bus. The pico can then translate "psram" accesses to sdram accesses, which would **allow the pico to natively access sdram in the address space**. This might not be possible.

## Digilent Waveforms SDRAM Protocol Decoder
```js
// rgData: input, raw digital data array
// rgValue: output, decoded data array
// rgFlag: output, decoded flag array

c = rgData.length

for(var i = 0; i < c; i++){
   // take 4 least significant bits
   rgValue[i] = rgData[i] & 0xFF;
   // set flag other than zero for valid data
   rgFlag[i] = 1;
}

// value: value sample
// flag: flag sample
function Value2Text(flag, value) {
  var lower = value & 0xf;
  var addr10 = ((value >> 7) & 1) > 0;
  if (lower & 1) return "";
  if (lower == 14) return "NOP";
  if (lower == 6) return "BURST_TERM";
  if (lower == 10) return addr10 ? "READ_PRECHARGE" : "READ";
  if (lower == 2) return addr10 ? "WRITE_PRECHARGE" : "WRITE";
  if (lower == 12) return "ACTIVATE";
  if (lower == 4) return "PRECHARGE";
  if (lower == 8) return "AUTO_REFRESH";
  if (lower == 0) return "LOAD_MODE";
  return "X";
}
Value2Text(0,0x82)
```
