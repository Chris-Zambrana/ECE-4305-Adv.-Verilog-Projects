module chu_vga_radar_core (
   input  logic clk, reset,

   // frame counter / current pixel coordinate
   input  logic [10:0] x, y,

   // video slot interface
   input  logic        cs,
   input  logic        write,
   input  logic [13:0] addr,
   input  logic [31:0] wr_data,

   // stream interface
   input  logic [11:0] si_rgb,
   output logic [11:0] so_rgb
);

   // ------------------------------------------------------------
   // Register map
   // ------------------------------------------------------------
   localparam logic [13:0] BYPASS_REG          = 14'h2000;
   localparam logic [13:0] X0_REG              = 14'h2001;
   localparam logic [13:0] Y0_REG              = 14'h2002;
   localparam logic [13:0] DISTANCE_REG        = 14'h2003;
   localparam logic [13:0] ANGLE_REG           = 14'h2004;
   localparam logic [13:0] COLOR_REG           = 14'h2005;
   localparam logic [13:0] MODE_REG            = 14'h2006;
   localparam logic [13:0] OBJECT_DETECTED_REG = 14'h2007;
   localparam logic [13:0] THICKNESS_REG       = 14'h2008;
   localparam logic [13:0] FADE_REG            = 14'h2009;

   // ------------------------------------------------------------
   // Stored software-control registers
   // ------------------------------------------------------------
   logic        bypass_reg;

   logic [10:0] x0_reg;
   logic [10:0] y0_reg;

   logic [7:0]  distance_reg;
   logic [8:0]  angle_reg;
   logic [11:0] color_reg;
   logic [2:0]  mode_reg;
   logic        object_detected_reg;

   logic [31:0] thickness_reg;
   logic [31:0] fade_reg;

   // ------------------------------------------------------------
   // Decoded config fields
   // ------------------------------------------------------------
   logic [3:0] sweep_thickness;
   logic [3:0] object_thickness;
   logic [3:0] fade_thickness;

   logic [3:0] fade_levels;
   logic [3:0] fade_step;
   logic       fade_enable;

   // ------------------------------------------------------------
   // Internal signals
   // ------------------------------------------------------------
   logic        wr_en;
   logic [11:0] radar_rgb;

   assign wr_en = cs & write;

   // thickness_config register layout:
   // bits [3:0]   = sweep_thickness
   // bits [7:4]   = object_thickness
   // bits [11:8]  = fade_thickness
   assign sweep_thickness  = thickness_reg[3:0];
   assign object_thickness = thickness_reg[7:4];
   assign fade_thickness   = thickness_reg[11:8];

   // fade_config register layout:
   // bits [3:0] = fade_levels
   // bits [7:4] = fade_step
   // bit  [8]   = fade_enable
   assign fade_levels = fade_reg[3:0];
   assign fade_step   = fade_reg[7:4];
   assign fade_enable = fade_reg[8];

   // ------------------------------------------------------------
   // Register write logic
   // ------------------------------------------------------------
   always_ff @(posedge clk, posedge reset) begin
      if (reset) begin
         // Default to bypass enabled for safety.
         bypass_reg          <= 1'b1;

         // Default radar geometry
         x0_reg              <= 11'd320;
         y0_reg              <= 11'd240;

         // Default scan state
         distance_reg        <= 8'd200;
         angle_reg           <= 9'd0;
         color_reg           <= 12'h0F0;
         mode_reg            <= 3'd0;
         object_detected_reg <= 1'b0;

         // Default visual config:
         // sweep_thickness  = 3
         // object_thickness = 3
         // fade_thickness   = 3
         thickness_reg       <= 32'h0000_0333;

         // Default fade config:
         // fade_levels = 4
         // fade_step   = 2
         // fade_enable = 1
         fade_reg            <= 32'h0000_0124;
      end else begin
         if (wr_en) begin
            case (addr)
               BYPASS_REG: begin
                  bypass_reg <= wr_data[0];
               end

               X0_REG: begin
                  x0_reg <= wr_data[10:0];
               end

               Y0_REG: begin
                  y0_reg <= wr_data[10:0];
               end

               DISTANCE_REG: begin
                  distance_reg <= wr_data[7:0];
               end

               ANGLE_REG: begin
                  angle_reg <= wr_data[8:0];
               end

               COLOR_REG: begin
                  color_reg <= wr_data[11:0];
               end

               MODE_REG: begin
                  mode_reg <= wr_data[2:0];
               end

               OBJECT_DETECTED_REG: begin
                  object_detected_reg <= wr_data[0];
               end

               THICKNESS_REG: begin
                  thickness_reg <= wr_data;
               end

               FADE_REG: begin
                  fade_reg <= wr_data;
               end

               default: begin
                  // Unmapped addresses do not modify registers.
               end
            endcase
         end
      end
   end

   // ------------------------------------------------------------
   // Radar overlay engine
   // ------------------------------------------------------------
   // This module will generate the processed radar-overlay pixel.
   // Top-level bypass mux below chooses between original stream pixel
   // and radar-processed pixel, matching your existing core style.
   // ------------------------------------------------------------
   radar_overlay_engine radar_overlay_engine_unit (
      .clk              (clk),
      .reset            (reset),

      .x                (x),
      .y                (y),
      .si_rgb           (si_rgb),
      .so_rgb           (radar_rgb),

      .x0               (x0_reg),
      .y0               (y0_reg),
      .distance         (distance_reg),
      .angle            (angle_reg),
      .color            (color_reg),
      .mode             (mode_reg),
      .object_detected  (object_detected_reg),

      .sweep_thickness  (sweep_thickness),
      .object_thickness (object_thickness),
      .fade_thickness   (fade_thickness),

      .fade_levels      (fade_levels),
      .fade_step        (fade_step),
      .fade_enable      (fade_enable)
   );

   // ------------------------------------------------------------
   // Top-level bypass mux
   // ------------------------------------------------------------
   assign so_rgb = bypass_reg ? si_rgb : radar_rgb;

endmodule