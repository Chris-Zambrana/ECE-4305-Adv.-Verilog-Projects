`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/23/2026 12:03:16 PM
// Design Name: 
// Module Name: mini_mem_block
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


module mini_mem_block(
    input logic clk, 
    input logic we, //write enable
    input logic [9:0] addr,
    input logic [3:0] din,
    output logic [3:0] dout
    );

    logic [3 : 0] memory [0: 2 ** 10 - 1];
    
    always_ff @(posedge clk)
    begin
        // write operation
        if (we)
            memory[addr] <= din;
        
        // read operation
        dout <= memory[addr];
    end

endmodule
