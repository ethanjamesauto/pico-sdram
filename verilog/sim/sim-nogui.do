vlib work
vlog ../IS42VM16400K.v ../MT48LC8M16A2.v ../test_sdram.sv ../74595.v
vopt test_sdram -o test_sdram_opt
vsim test_sdram_opt
run -all
exit