module blinking_led_pulse
#(
    parameter int COUNTER_WIDTH = 33
)
(
    input  logic                 clk,
    input  logic                 reset,
    input  logic [COUNTER_WIDTH-1:0] max_count,
    output logic                 dout  
);

    logic [COUNTER_WIDTH-1:0] count_reg, count_next;
    logic tick;

    t_flip_flop tff (
        .clk(clk),
        .reset(reset),
        .t(tick),
        .out(dout)
    );
    
    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            count_reg <= '0;
        end else begin
            count_reg <= count_next;
        end
    end

    always_comb begin
        count_next = count_reg;
        if (count_reg == max_count-1) 
        begin
            count_next = 0;     
        end 
        else 
        begin
            count_next = count_reg + 1;
        end
    end
    
    assign tick = (count_reg == max_count-1) ? 1 : 0;

endmodule
