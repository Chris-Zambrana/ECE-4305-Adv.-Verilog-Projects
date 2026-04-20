`timescale 1ns / 1ps

module dout_mux(
    input logic [3:0] dout0, dout1, dout2, dout3,
    input logic [1:0] sel,
    output logic [3:0] dout
    );

    always_comb begin 
        case (sel)
            2'b00: dout = dout0;
            2'b01: dout = dout1;
            2'b10: dout = dout2;
            2'b11: dout = dout3;
            default: dout = 4'b0000; // default case to avoid latches
        endcase
    end

endmodule
