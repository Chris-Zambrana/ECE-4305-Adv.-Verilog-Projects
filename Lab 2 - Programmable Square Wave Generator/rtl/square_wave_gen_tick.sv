`timescale 1ns / 1ps

module square_wave_gen_tick
#(
    parameter int NEW_WIDTH = 8
)
(
    input  logic [NEW_WIDTH-1:0] target,
    input logic [NEW_WIDTH-1:0] count,     
    output logic                tick
);

assign tick = (count == target-1) ? 1 : 0;

endmodule