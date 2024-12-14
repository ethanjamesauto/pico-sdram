`timescale 1ns / 1ps

// model of the pico-sdram board
module test_sdram_real;

wire [15:0] dq;
logic [12:0] addr;
logic [1:0] ba;
logic clk;
logic cke;
logic csb;
logic rasb;
logic casb;
logic web;
logic [1:0] dqm;

logic srclk, /*clk*/ rclk, ser1, ser2, ser3;

MT48LC8M16A2 sdram(
    .dq(dq),
    .addr(addr),
    .ba(ba),
    .clk(clk),
    .cke(cke),
    .csb(csb),
    .rasb(rasb),
    .casb(casb),
    .web(web),
    .dqm(dqm)
);

logic dq_en = 0;
logic [15:0] dq_in;
assign dq = dq_en ? dq_in : 16'bZ;

initial begin
    $display("Starting simulation");
    addr = 13'b0;
    ba = 2'b00;
    clk = 1'b0;
    cke = 1'b1;
    csb = 1'b1;
    rasb = 1'b0;
    casb = 1'b0;
    web = 1'b1;
    dqm = 2'b11;
    #1us;
end



endmodule