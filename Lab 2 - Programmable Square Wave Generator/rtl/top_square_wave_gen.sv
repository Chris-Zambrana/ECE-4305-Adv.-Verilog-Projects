`timescale 1ns / 1ps

module top_square_wave_gen
#(
    parameter int WIDTH = 4,
    parameter int NEW_WIDTH = 8
)
(
    input  logic clk,
    input  logic reset,
    input  logic [WIDTH-1:0] m,
    input  logic [WIDTH-1:0] n,
    output logic out
);

    logic tick;
    logic [NEW_WIDTH-1:0]     m10;
    logic [NEW_WIDTH-1:0]     n10;
    logic [NEW_WIDTH-1:0]     target;
    logic [NEW_WIDTH-1:0]     count;

    square_wave_gen_new_num #(WIDTH,NEW_WIDTH) target_nums (m,n,m10,n10);
    
    square_wave_gen_target #(.NEW_WIDTH(NEW_WIDTH)) target_mux (.m10(m10),.n10(n10),.state(out),.target(target));
    
    square_wave_gen_count #(.NEW_WIDTH(NEW_WIDTH)) counter (
        .clk   (clk),
        .reset (reset),
        .target (target),     
        .count  (count)
    );
    
    square_wave_gen_tick #(NEW_WIDTH) ticker (target, count, tick);
    
    t_flip_flop flop0 (
        .clk   (clk),
        .reset (reset),
        .t     (tick),
        .out   (out)
    );

endmodule
