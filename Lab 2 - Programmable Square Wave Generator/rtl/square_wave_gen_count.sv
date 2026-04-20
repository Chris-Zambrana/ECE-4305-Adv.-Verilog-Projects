`timescale 1ns / 1ps

module square_wave_gen_count
#(
    parameter int NEW_WIDTH = 8
)
(
    input  logic                 clk,
    input  logic                 reset,
    input  logic [NEW_WIDTH-1:0] target,
    output logic [NEW_WIDTH-1:0] count     
);

    logic [NEW_WIDTH-1:0] count_reg, count_next;
    
    always_ff @(posedge clk) begin
        if (reset) begin
            count_reg <= '0;
        end else begin
            count_reg <= count_next;
        end
    end

    always_comb begin
        count_next = count_reg;
        if (count_reg == target-1) begin
            count_next = 0;     
        end else begin
            count_next = count_reg + 1;
        end
    end
    
    assign count = count_reg;

endmodule
