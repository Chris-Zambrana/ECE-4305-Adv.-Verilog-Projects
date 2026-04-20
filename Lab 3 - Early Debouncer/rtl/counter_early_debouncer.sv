`timescale 1ns / 1ps

module counter_early_debouncer
#(
    parameter MAX_COUNT = 1000000
)
(
        input logic clk, // genrally the 100MHz
        input logic reset,
        output logic tick
);
    localparam COUNTER_WIDTH = $clog2(MAX_COUNT);
    logic [COUNTER_WIDTH-1:0] count_reg, count_next;
    
       

    always_ff @(posedge clk) begin
        if(reset) begin
            count_reg <= 0;
        end else begin
            count_reg <= count_next;
        end
    end
    
    always_comb begin
        count_next = count_reg;
        tick = 0;
        if(count_reg == MAX_COUNT-1) begin
            tick = 1;
            count_next = 0;
        end else begin
            tick = 0;
            count_next = count_reg + 1;
        end
    end

endmodule
