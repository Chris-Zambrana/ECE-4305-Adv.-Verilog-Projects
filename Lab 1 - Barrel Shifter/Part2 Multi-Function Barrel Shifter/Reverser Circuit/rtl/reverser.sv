`timescale 1ns / 1ps

module reverser
#(
    parameter N = 3,
    localparam int w = (1 << N)
)
(
    input logic [w-1:0] in,
    input logic lr,
    output logic [w-1:0] reverse_out
);

    integer i = 0;
    
    always_comb begin
        if (lr) begin
            for(i=0; i < w; i = i + 1) begin : reverse
                reverse_out[i] = in[w-1-i];
            end
        end else begin
            reverse_out = in;
        end
    end

endmodule
