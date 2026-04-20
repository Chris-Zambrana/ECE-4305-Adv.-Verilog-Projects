`timescale 1ns / 1ps

module top_asymmetric_fifo
    #(parameter ADDR_WIDTH = 3, DATA_WIDTH = 8)
    (
        input logic clk, reset,
        input logic wr, rd,
        input logic [DATA_WIDTH*2 - 1: 0] w_data,
        output logic [DATA_WIDTH - 1: 0] r_data,
        output logic full, empty
    );
    
    // signal declaration
    logic [ADDR_WIDTH - 1: 0] w_addr, r_addr;
    
    // instantiate register file
    reg_file #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH))
        r_file_unit (.w_en( wr & ~full), .*);
    
    // instantiate fifo controller
    fifo_ctrl #(.ADDR_WIDTH(ADDR_WIDTH))
        ctrl_unit (.*);                    
endmodule
