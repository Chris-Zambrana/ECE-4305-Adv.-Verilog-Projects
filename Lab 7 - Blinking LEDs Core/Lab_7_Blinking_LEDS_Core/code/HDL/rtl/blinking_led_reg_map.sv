module blinking_led_reg_map
(
    // slot interface
    input logic clk,
    input logic reset,
    input logic cs,
    input logic write,
    input logic [31:0] wr_data,
    input logic [4:0] addr,
    // external output  
    output logic [31:0] blinking_led_reg [3:0] // there are 32 registers so in reality its [32:0] but we will only use the first 4 for max_count
);

// declaration
   logic wr_en;

   always_ff @(posedge clk, posedge reset)
   begin 
      if(reset)
      begin
         blinking_led_reg[0] <= 32'b0;
         blinking_led_reg[1] <= 32'b0;
         blinking_led_reg[2] <= 32'b0;
         blinking_led_reg[3] <= 32'b0;
      end
      else if(wr_en)
      begin
         case(addr)
            0: blinking_led_reg[0] <= {{16{1'b0}}, wr_data[15:0]}; // write to reg0
            1: blinking_led_reg[1] <= {{16{1'b0}}, wr_data[15:0]}; // write to reg1
            2: blinking_led_reg[2] <= {{16{1'b0}}, wr_data[15:0]}; // write to reg2
            3: blinking_led_reg[3] <= {{16{1'b0}}, wr_data[15:0]}; // write to reg3
         endcase
      end 
   end

   assign wr_en = cs && write && (addr == 0 || addr == 1 || addr == 2 || addr == 3); // only allow writes to the first two registers for max_count
    
endmodule