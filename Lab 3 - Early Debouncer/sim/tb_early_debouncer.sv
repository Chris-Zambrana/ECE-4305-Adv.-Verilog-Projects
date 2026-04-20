`timescale 1ns / 1ps

module tb_early_debouncer;

    localparam MAX_COUNT = 10;
    localparam time CLK_PERIOD = 10ns;    

    logic clk; // genrally the 100MHz
    logic reset;
    logic sw;
    logic db;
    
    top_early_debouncer #(MAX_COUNT) dut
    (
            clk, // genrally the 100MHz
            reset,
            sw,
            db
    );

    always #(CLK_PERIOD/2) clk = ~clk;
    
    initial begin
        
        clk=0;reset=0;sw=0;
        
        @(posedge clk);
        reset = 1;
        @(posedge clk);
        reset = 0;
        repeat(2)@(posedge clk);
        sw=1;
        repeat(6) begin
            @(posedge clk);
            sw = ~sw;
        end
        
        wait(dut.tick);
        
        #350 sw = 0;
        @(posedge clk);
        repeat(6) begin
            @(posedge clk);
            sw = ~sw;
        end
        
        wait(dut.tick);
        
        #350 sw = 1;
        
        #500 $finish;        
    end

endmodule
