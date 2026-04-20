`timescale 1ns / 1ps

module multi_barrel_shifter_reverser
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

    logic [w-1:0] reverse_out_1, right_shift_out;
    
    reverser #(.N(N)) reverse_1 (.in(in), .lr(lr), .reverse_out(reverse_out_1));
    param_right_shifter #(.N(N)) right_shift (.in(reverse_out_1), .amt(amt), .right_out(right_shift_out));
    reverser #(.N(N)) reverse_2 (.in(right_shift_out), .lr(lr), .reverse_out(out));

endmodule
