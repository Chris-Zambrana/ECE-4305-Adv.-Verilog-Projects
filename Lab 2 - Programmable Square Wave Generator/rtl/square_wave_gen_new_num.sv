`timescale 1ns / 1ps

module square_wave_gen_new_num
#(
    parameter int WIDTH = 4,
    parameter int NEW_WIDTH = 8
)
(
        input  logic [WIDTH-1:0]     m,
        input  logic [WIDTH-1:0]     n,
        output  logic [NEW_WIDTH-1:0]     m10,
        output  logic [NEW_WIDTH-1:0]     n10
);
    
    assign m10 = m*10;
    assign n10 = n*10;
    
endmodule
