`timescale 1ns / 1ps

module multi_barrel_shifter_reverser_tb;

    parameter N = 3;
    localparam int w = (1 << N);
    logic [w-1:0] in;
    logic [N-1:0] amt;
    logic lr;
    logic [w-1:0] out;
    
    multi_barrel_shifter_reverser #(.N(N)) dut (in, amt, lr, out);
    
    initial begin
        
        in = 0; amt = 0; lr = 0;
        
        // right shift
        #5 in = 100; amt = 5; 
        #5 in = 7; amt = 6; 
        #5 in = 245; amt = 1; 
        
        // reset
        #5 in = 0; amt = 0; lr = 0;
        
        // left shift
        #5 in = 35;  amt = 2; lr = 1;
        #5 in = 79;  amt = 7; lr = 1;
        #5 in = 120;  amt = 4; lr = 1;
    
        #5 $finish; 
    end

endmodule
