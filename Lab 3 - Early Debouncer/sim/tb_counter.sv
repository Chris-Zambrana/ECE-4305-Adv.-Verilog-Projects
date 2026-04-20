`timescale 1ns / 1ps

module tb_counter;

    localparam MAX_COUNT = 10;
    localparam time CLK_PERIOD = 10ns;

    logic clk; // genrally the 100MHz
    logic reset;
    logic start;
    logic tick;
    
    counter_early_debouncer #(MAX_COUNT) dut (clk,reset,start,tick);

    always #(CLK_PERIOD/2) clk = ~clk;
    
    initial begin
        
        clk=0;start=0;reset=0;
        
        @(posedge clk);
        reset = 1;
        @(posedge clk);
        reset = 0;
        
        repeat(2) @(posedge clk);
        start = 1;
        @(posedge clk);
        start = 0;
        
        wait(tick);
        repeat(5) @(posedge clk);
        start = 1;
        @(posedge clk);
        start = 0;
        
    end
    
endmodule
