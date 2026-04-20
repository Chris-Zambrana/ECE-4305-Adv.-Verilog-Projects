`timescale 1ns / 1ps

module param_lr_shifter_tb;
    parameter N = 3;
    localparam int w = (1 << N);
    

    logic [w-1:0] in;
    logic [N-1:0] amt;
    logic [w-1:0] right_out;
    logic [w-1:0] left_out;
    
    param_left_shifter #(N) dut0 (in, amt, left_out);
    param_right_shifter #(N) dut1 (in, amt, right_out);

    initial begin
        
        in = 0; amt = 0; 
        
        // right shift
        #5 in = 100; amt = 5; 
        #5 in = 7; amt = 6; 
        #5 in = 245; amt = 1; 
        
        // reset
        #5 in = 0; amt = 0; 
        
        // left shift
        #5 in = 35;  amt = 2; 
        #5 in = 79;  amt = 7; 
        #5 in = 120;  amt = 4;
    
        #5 $finish; 
    end

endmodule
