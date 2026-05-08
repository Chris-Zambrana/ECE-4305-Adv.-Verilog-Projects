`timescale 1ns / 1ps

module top_rom_temp_conv 
#(
    parameter DATA_WIDTH = 8,
    parameter ADDRESS_BITS = 9,
    parameter DEC_DIGITS = (DATA_WIDTH*30103 + 99999) / 100000,
    parameter BCD_WIDTH = DEC_DIGITS * 4
)
(
    input logic clk,
    input logic rst,
    input logic [DATA_WIDTH-1:0] din,
    input logic format,
    output logic [6:0] sseg,
    output logic dp,
    output logic [7:0] an
);

logic [ADDRESS_BITS-1:0] addr = {format, din};
logic [DATA_WIDTH-1:0] data_out;
logic [BCD_WIDTH-1:0] bcd_out0, bcd_out1;

logic [5:0] input_temp_symbol = format ? 6'b111111: 6'b111001; 
logic [5:0] output_temp_symbol = format ? 6'b111001: 6'b111111; 


synch_rom #(.DATA_BITS(DATA_WIDTH), .ADDRESS_BITS(ADDRESS_BITS)) rom0
(
    .clk(clk),
    .addr(addr),
    .data(data_out)
);

binary_to_bcd #(.BINARY_WIDTH(DATA_WIDTH), .BCD_WIDTH(BCD_WIDTH), .DEC_DIGITS(DEC_DIGITS)) bcd_converter0
(
    .binary_in(din),
    .bcd_out(bcd_out0)
);

binary_to_bcd #(.BINARY_WIDTH(DATA_WIDTH), .BCD_WIDTH(BCD_WIDTH), .DEC_DIGITS(DEC_DIGITS)) bcd_converter1
(
    .binary_in(data_out),
    .bcd_out(bcd_out1)
);

time_mux_disp disp 
(
    .in0(input_temp_symbol), 
    .in1({1'b1 , bcd_out0[3:0], 1'b1}),
    .in2({1'b1 , bcd_out0[7:4], 1'b1}),
    .in3({1'b1 , bcd_out0[11:8], 1'b1}),
    .in4(output_temp_symbol),
    .in5({1'b1 , bcd_out1[3:0], 1'b1}),
    .in6({1'b1 , bcd_out1[7:4], 1'b1}),
    .in7({1'b1 , bcd_out1[11:8], 1'b1}),
    .dp(dp),
    .*
);

endmodule
