
module chu_blinking_led 
#(parameter W = 8)  // width of output port
(
    input logic clk,
    input logic reset,
    // slot interface
    input logic cs,
    input logic read, // not used
    input logic write,
    input logic [4:0] addr,
    input logic [31:0] wr_data,
    output logic [31:0] rd_data, // not used
    // external output  
    output logic [W-1:0] dout
);

localparam COUNTER_WIDTH = 33; // 32 bits gives us about 1.6 seconds at 100 MHz

// there are 32 registers so in reality its [32:0] but we will only use the first 4 for max_count
logic [31:0] blinking_led_reg [3:0]; // 32-bit registers for each of the 32 addresses (0-4)
logic [COUNTER_WIDTH-1:0] max_count [3:0]; // max_count for each timer; only use first 4 entries`
logic dout0, dout1, dout2, dout3;

assign max_count[0] = blinking_led_reg[0] * 100000; // lower 32 bits of reg0
assign max_count[1] = blinking_led_reg[1] * 100000; // lower 32 bits of reg1
assign max_count[2] = blinking_led_reg[2] * 100000; // lower 32 bits of reg2
assign max_count[3] = blinking_led_reg[3] * 100000; // lower 32 bits of reg3

blinking_led_reg_map reg_decoder(
    .clk(clk),
    .reset(reset),
    .cs(cs),
    .write(write),
    .wr_data(wr_data),
    .addr(addr),
    .blinking_led_reg(blinking_led_reg)
);

blinking_led_pulse
#(
    .COUNTER_WIDTH (COUNTER_WIDTH )
)
led_timer0(
    .clk       (clk       ),
    .reset     (reset     ),
    .max_count (max_count[0] ),
    .dout      (dout0      )
);

blinking_led_pulse
#(
    .COUNTER_WIDTH (COUNTER_WIDTH )
)
led_timer1(
    .clk       (clk       ),
    .reset     (reset     ),
    .max_count (max_count[1] ),
    .dout      (dout1      )
);

blinking_led_pulse
#(
    .COUNTER_WIDTH (COUNTER_WIDTH )
)
led_timer2(
    .clk       (clk       ),
    .reset     (reset     ),
    .max_count (max_count[2] ),
    .dout      (dout2      )
);

blinking_led_pulse
#(
    .COUNTER_WIDTH (COUNTER_WIDTH )
)
led_timer3(
    .clk       (clk       ),
    .reset     (reset     ),
    .max_count (max_count[3] ),
    .dout      (dout3      )
);

assign dout = {{(W-4){1'b0}}, dout3, dout2, dout1, dout0};


endmodule
    
