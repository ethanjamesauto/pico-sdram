vlib work
vlog IS42VM16400K.v MT48LC8M16A2.v test_sdram.sv
vsim test_sdram
# add wave *
add wave -r  sdram/* 
run 20us