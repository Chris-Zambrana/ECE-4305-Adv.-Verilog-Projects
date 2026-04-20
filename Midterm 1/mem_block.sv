`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/27/2022 07:29:58 PM
// Design Name: 
// Module Name: mem_block
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module mem_block(
    input logic clk, 
    input logic we, //write enable
    input logic [11:0] addr,
    input logic [3:0] din,
    output logic [3:0] dout
    );
    
    // TODO: Write your code here 
    // DO NOT CHANGE THE MODULE INTERFACE

    logic [3 : 0] dout0, dout1, dout2, dout3;
    logic we0, we1, we2, we3;

    we_mux u_we_mux(
        .we  (we  ),
        .sel (addr[11:10] ), // use the upper 2 bits of the address to select the mini memory block
        .we0 (we0 ),
        .we1 (we1 ),
        .we2 (we2 ),
        .we3 (we3 )
    );
    

    dout_mux u_dout_mux(
        .dout0 (dout0 ),
        .dout1 (dout1 ),
        .dout2 (dout2 ),
        .dout3 (dout3 ),
        .sel   (addr[11:10] ), // use the upper 2 bits of the address to select the mini memory block),
        .dout  (dout  )
    );

    mini_mem_block mini_mem0(
    .clk  (clk  ),
    .we   (we0   ),
    .addr (addr[9:0] ), // fix address width
    .din  (din  ),
    .dout (dout0 )
    );

    mini_mem_block mini_mem1(
    .clk  (clk  ),
    .we   (we1   ),
    .addr (addr[9:0] ), // fix address width
    .din  (din  ),
    .dout (dout1 )
    );

    mini_mem_block mini_mem2(
    .clk  (clk  ),
    .we   (we2   ),
    .addr (addr[9:0] ), // fix address width
    .din  (din  ),
    .dout (dout2 )
    );

    mini_mem_block mini_mem3(
    .clk  (clk  ),
    .we   (we3   ),
    .addr (addr[9:0] ), // fix address width
    .din  (din  ),
    .dout (dout3 )
    );

endmodule
