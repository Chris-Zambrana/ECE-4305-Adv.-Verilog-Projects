`timescale 1ns / 1ps

module square_wave_gen_target
#(
    parameter int NEW_WIDTH = 8
)
(
    input  logic [NEW_WIDTH-1:0]     m10,
    input  logic [NEW_WIDTH-1:0]     n10,
    input  logic                     state,
    output logic [NEW_WIDTH-1:0]     target
);
    
    assign target = state ? m10 : n10;
    
endmodule