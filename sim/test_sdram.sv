`timescale 1ns / 1ps

module test_sdram;

/*
module MT48LC8M16A2 (dq, addr, ba, clk, cke, csb, rasb, casb, web, dqm);

    parameter addr_bits =      13;
    parameter data_bits =      16;
    parameter col_bits  =       9;
    parameter mem_sizes = 2097151;                                  // 2 Meg
 
    inout     [data_bits - 1 : 0] dq;
    input     [addr_bits - 1 : 0] addr;
    input                 [1 : 0] ba;
    input                         clk;
    input                         cke;
    input                         csb;
    input                         rasb;
    input                         casb;
    input                         web;
    input                 [1 : 0] dqm;
*/

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


initial forever #0.2us clk = ~clk;

task cmd_inhibit;
    @(negedge clk) begin
        $display("cmd inhibit");
        csb = 1'b1;
        rasb = 1'b1;
        casb = 1'b1;
        web = 1'b1;
        dqm = 2'b00;
        addr = 13'b0;
        ba = 2'b00;
    end
endtask

task activate;
    @(negedge clk) begin
        $display("Activate row 0");
        addr = 13'b0;
        ba = 2'b00;
        csb = 1'b0;
        rasb = 1'b0;
        casb = 1'b1;
        web = 1'b1;
        dqm = 2'b00;
    end
endtask

task read_with_auto_precharge;
    @(negedge clk) begin
        $display("Read row 0");
        addr = 13'b0;
        addr[10] = 1'b1;
        ba = 2'b00;
        csb = 1'b0;
        rasb = 1'b1;
        casb = 1'b0;
        web = 1'b1;
        dqm = 2'b00;
    end
endtask

task write_with_auto_precharge;
    @(negedge clk) begin
        $display("Write row 0");
        addr = 13'b0;
        addr[10] = 1'b1;
        ba = 2'b00;
        csb = 1'b0;
        rasb = 1'b1;
        casb = 1'b0;
        web = 1'b0;
        dqm = 2'b00;
    end
endtask

task write_mode_register;
    @(negedge clk) begin
        $display("Write mode register");
        addr = 13'b0;
        addr[2:0] = 3'b000; // burst length 1
        addr[3] = 0; // sequential
        addr[6:4] = 3'b010; // CAS latency 2
        addr[9] = 1; // burst read, single write
        ba = 2'b00;
        csb = 1'b0;
        rasb = 1'b0;
        casb = 1'b0;
        web = 1'b0;
        dqm = 2'b00;
    end
endtask

task precharge;
    @(negedge clk) begin
        $display("Precharge all");
        addr = 13'b0;
        addr[10] = 1'b1;
        ba = 2'b00;
        csb = 1'b0;
        rasb = 1'b0;
        casb = 1'b1;
        web = 1'b0;
        dqm = 2'b00;
    end
endtask


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

    cmd_inhibit;
    precharge;
    cmd_inhibit;

    write_mode_register;
    cmd_inhibit;

    /*
    activate;
    cmd_inhibit;
    write_with_auto_precharge;
    dq_en = 1;
    dq_in = 16'h1234;
    cmd_inhibit;
    dq_en = 0;
    //*/

    activate;
    cmd_inhibit;
    read_with_auto_precharge;
    cmd_inhibit;
end


endmodule