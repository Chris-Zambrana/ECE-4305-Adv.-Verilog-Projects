`timescale 1ns / 1ps

module param_left_shifter
#(
    parameter N = 3,
    localparam int w = (1 << N)
)
(
    input logic [w-1:0] in,
    input logic [N-1:0] amt,
    output logic [w-1:0] left_out
);

    integer i = 0;
    logic [w-1:0] shift;
    
    always_comb begin
        shift = in;
        for(i=0; i < amt; i = i + 1) begin : reverse
            shift = {shift[w-2:0], shift[w-1]};
        end
        left_out = shift;
    end
    
endmodule
