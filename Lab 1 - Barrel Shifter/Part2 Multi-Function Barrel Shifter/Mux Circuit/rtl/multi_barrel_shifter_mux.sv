`timescale 1ns / 1ps

module multi_barrel_shifter_mux
#(
    parameter N = 3,
    localparam int w = (1 << N)
)
(
    input logic [w-1:0] in,
    input logic [N-1:0] amt,
    input logic lr,
    output logic [w-1:0] out
);
    logic [w-1:0] right_out, left_out;
    
    param_right_shifter #(N) right_shift(in, amt, right_out);
    param_left_shifter #(N) left_shift(in, amt, left_out);
    
    always_comb begin
        out = lr ? left_out : right_out;
    end
    
endmodule
