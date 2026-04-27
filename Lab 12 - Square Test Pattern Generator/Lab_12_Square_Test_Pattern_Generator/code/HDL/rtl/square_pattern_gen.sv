module square_pattern_gen

    #(parameter CD = 12)    // color depth
   (
    input  logic [10:0] x, y,     // treated as x-/y-axis
    input  logic [CD-1:0] rgb,       // color input
    input  logic [1:0] square_size, // 4 different sizes of square pattern
    output logic [CD-1:0] square_rgb 
   );

   // declaration
   logic [3:0] up, down;
   logic [3:0] r, g, b;
   logic [3:0] r_out, g_out, b_out;
   logic [7:0] size;
   
   assign r = rgb[3:0];
   assign g = rgb[7:4];
   assign b = rgb[11:8];

   always_comb begin
    case(square_size)
        2'b00: begin
            size = 16;
        end
        2'b01: begin
            size = 32;
        end
        2'b10: begin
            size = 64;
        end
        2'b11: begin
            size = 128;
        end
        default: begin
            size = 16;
        end
    endcase
   end
    
   always_comb
   begin
      if((x >= (320 - size/2) && x < (320 + size/2)) && (y >= (240 - size/2) && y < (240 + size/2))) begin
         r_out = r;
         g_out = g;
         b_out = b;
      end
      else begin
         r_out = ~r;
         g_out = ~g;
         b_out = ~b;
      end
   end // always   
   // output
   assign square_rgb = {b_out, g_out, r_out};
endmodule