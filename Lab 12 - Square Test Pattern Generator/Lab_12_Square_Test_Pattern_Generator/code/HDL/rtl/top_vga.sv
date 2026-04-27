module top_vga
   #(parameter CD = 12)    // color depth
   (
    input  logic clk,
    input  logic [13:0] sw,
    // to vga monitor
    output logic hsync, vsync,
    output logic[CD-1:0] rgb
   );

   // logic [CD-1:0] declaration
   logic [10:0] hc, vc;
   logic [CD-1:0] square_rgb;
   logic [1:0] square_size;
   
   // body
   // use switches to set background color
   assign square_size = sw[13:12];
   // instantiate square pattern generator
   square_pattern_gen #(.CD(CD)) square_unit
      (.x(hc), .y(vc), .rgb(sw[11:0]), .square_size(square_size), .square_rgb(square_rgb));
   // instantiate color-to-gray conversion circuit

   vga_sync_demo #(.CD(CD)) sync_unit
      (.clk(clk), .reset(0), .vga_si_rgb(square_rgb),
       .hsync(hsync), .vsync(vsync), .rgb(rgb), .hc(hc), .vc(vc));

endmodule