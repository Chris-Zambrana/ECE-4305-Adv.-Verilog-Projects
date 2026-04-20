`timescale 1ns / 1ps


module we_mux(
        input logic we,
        input logic [1:0] sel,
        output logic we0, we1, we2, we3
    );

    always_comb begin 
        case (sel)
            2'b00: {we0, we1, we2, we3} = {we, 1'b0, 1'b0, 1'b0};
            2'b01: {we0, we1, we2, we3} = {1'b0, we, 1'b0, 1'b0};
            2'b10: {we0, we1, we2, we3} = {1'b0, 1'b0, we, 1'b0};
            2'b11: {we0, we1, we2, we3} = {1'b0, 1'b0, 1'b0, we};
            default: {we0, we1, we2, we3} = {4{1'b0}};
        endcase
    end

endmodule
