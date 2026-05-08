`timescale 1ns / 1ps

module binary_to_bcd
#(
    parameter BINARY_WIDTH = 8,
    parameter BCD_WIDTH = 12,
    parameter DEC_DIGITS = 3
)
(
    input logic [BINARY_WIDTH-1:0] binary_in,
    output logic [BCD_WIDTH-1:0] bcd_out
);

integer i, d;

always_comb begin
    bcd_out = '0;

    for (i = 0; i < BINARY_WIDTH; i = i + 1) begin
        // Add-3 correction on each BCD nibble
        for (d = 0; d < DEC_DIGITS; d = d + 1) begin
            if (bcd_out[d*4 +: 4] >= 5)
                bcd_out[d*4 +: 4] = bcd_out[d*4 +: 4] + 3;
        end

        // Shift left and bring in next binary bit (MSB first)
        bcd_out = {bcd_out[BCD_WIDTH-2:0], binary_in[BINARY_WIDTH-1-i]};
    end
end

endmodule
