`timescale 1ns / 1ps

module radar_overlay_engine #(
   parameter int HIST_DEPTH = 16
)(
   input  logic clk, reset,

   // current VGA pixel coordinate
   input  logic [10:0] x,
   input  logic [10:0] y,

   // incoming and outgoing RGB444 pixel stream
   input  logic [11:0] si_rgb,
   output logic [11:0] so_rgb,

   // control-register values from chu_vga_radar_core
   input  logic [10:0] x0,
   input  logic [10:0] y0,
   input  logic [7:0]  distance,
   input  logic [8:0]  angle,
   input  logic [11:0] color,
   input  logic [2:0]  mode,
   input  logic        object_detected,

   input  logic [3:0]  sweep_thickness,
   input  logic [3:0]  object_thickness,
   input  logic [3:0]  fade_thickness,

   input  logic [3:0]  fade_levels,
   input  logic [3:0]  fade_step,
   input  logic        fade_enable
);

   // ------------------------------------------------------------
   // Local constants
   // ------------------------------------------------------------

   localparam logic [2:0] MODE_180    = 3'd0;
   localparam logic [2:0] MODE_360    = 3'd1;
   localparam logic [2:0] MODE_MANUAL = 3'd2;

   localparam int RADAR_RADIUS_PIX = 200;
   localparam int RADAR_RADIUS_SQ  = 40000;

   localparam logic [7:0]  RADAR_RADIUS_U8 = 8'd200;
   localparam logic [11:0] OBJECT_RED      = 12'hF00;

   // ------------------------------------------------------------
   // Current angle sine/cosine
   // ------------------------------------------------------------
   //
   // Q8 signed format:
   //   +256 = +1.000
   //      0 =  0.000
   //   -256 = -1.000
   //
   // Angle convention:
   //   0 degrees   = right
   //   90 degrees  = up
   //   180 degrees = left
   //   270 degrees = down
   // ------------------------------------------------------------

   logic signed [9:0] cos_cur;
   logic signed [9:0] sin_cur;

   radar_angle_lut angle_lut_unit (
      .angle_deg(angle),
      .cos_q8   (cos_cur),
      .sin_q8   (sin_cur)
   );

   // ------------------------------------------------------------
   // Angle history for fading trail
   // ------------------------------------------------------------
   //
   // The history updates only when the angle changes.
   //
   // hist[0] = most recent previous/current angle sample
   // hist[1] = older angle sample
   // hist[2] = older angle sample
   // ...
   //
   // The active sweep and object beam use cos_cur/sin_cur directly.
   // The fade trail uses older history samples.
   // ------------------------------------------------------------

   logic [8:0] angle_prev;

   logic signed [9:0] cos_hist [0:HIST_DEPTH-1];
   logic signed [9:0] sin_hist [0:HIST_DEPTH-1];

   integer h;

   always_ff @(posedge clk, posedge reset) begin
      if (reset) begin
         angle_prev <= 9'd0;

         for (h = 0; h < HIST_DEPTH; h = h + 1) begin
            cos_hist[h] <= 10'sd256;
            sin_hist[h] <= 10'sd0;
         end
      end else begin
         if (angle != angle_prev) begin
            angle_prev <= angle;

            for (h = HIST_DEPTH-1; h > 0; h = h - 1) begin
               cos_hist[h] <= cos_hist[h-1];
               sin_hist[h] <= sin_hist[h-1];
            end

            cos_hist[0] <= cos_cur;
            sin_hist[0] <= sin_cur;
         end
      end
   end

   // ------------------------------------------------------------
   // Helper functions
   // ------------------------------------------------------------

   function automatic logic signed [31:0] abs_s32(
      input logic signed [31:0] value
   );
      begin
         if (value < 0)
            abs_s32 = -value;
         else
            abs_s32 = value;
      end
   endfunction

   function automatic logic [3:0] thickness_or_one(
      input logic [3:0] thickness
   );
      begin
         if (thickness == 4'd0)
            thickness_or_one = 4'd1;
         else
            thickness_or_one = thickness;
      end
   endfunction

   function automatic logic line_hit_test(
      input logic signed [12:0] dx,
      input logic signed [12:0] dy,
      input logic signed [9:0]  dir_cos,
      input logic signed [9:0]  dir_sin,
      input logic [7:0]         inner_radius,
      input logic [7:0]         outer_radius,
      input logic [3:0]         thickness
   );
      logic signed [31:0] along_q8;
      logic signed [31:0] cross_q8;
      logic signed [31:0] abs_cross_q8;

      logic signed [31:0] inner_q8;
      logic signed [31:0] outer_q8;
      logic signed [31:0] thick_q8;

      begin
         // along = distance projected along the sweep direction
         // cross = perpendicular distance from the sweep direction
         //
         // Since dir_cos and dir_sin are Q8, along_q8 and cross_q8
         // are also scaled by 256.

         along_q8 = (dx * dir_cos) + (dy * dir_sin);
         cross_q8 = (dx * dir_sin) - (dy * dir_cos);

         abs_cross_q8 = abs_s32(cross_q8);

         inner_q8 = $signed({24'd0, inner_radius}) <<< 8;
         outer_q8 = $signed({24'd0, outer_radius}) <<< 8;
         thick_q8 = $signed({28'd0, thickness_or_one(thickness)}) <<< 8;

         line_hit_test =
            (along_q8 >= inner_q8) &&
            (along_q8 <= outer_q8) &&
            (abs_cross_q8 <= thick_q8);
      end
   endfunction

   function automatic logic [11:0] alpha_blend_rgb444(
      input logic [11:0] bg_rgb,
      input logic [11:0] overlay_rgb,
      input logic [4:0]  alpha
   );
      logic [3:0] bg_r;
      logic [3:0] bg_g;
      logic [3:0] bg_b;

      logic [3:0] ov_r;
      logic [3:0] ov_g;
      logic [3:0] ov_b;

      logic [4:0] inv_alpha;

      logic [8:0] mix_r;
      logic [8:0] mix_g;
      logic [8:0] mix_b;

      logic [3:0] out_r;
      logic [3:0] out_g;
      logic [3:0] out_b;

      begin
         // alpha scale:
         //   alpha = 0  -> 0% overlay, 100% background
         //   alpha = 16 -> 100% overlay, 0% background

         inv_alpha = 5'd16 - alpha;

         bg_r = bg_rgb[11:8];
         bg_g = bg_rgb[7:4];
         bg_b = bg_rgb[3:0];

         ov_r = overlay_rgb[11:8];
         ov_g = overlay_rgb[7:4];
         ov_b = overlay_rgb[3:0];

         mix_r = (ov_r * alpha) + (bg_r * inv_alpha);
         mix_g = (ov_g * alpha) + (bg_g * inv_alpha);
         mix_b = (ov_b * alpha) + (bg_b * inv_alpha);

         out_r = mix_r[8:4];
         out_g = mix_g[8:4];
         out_b = mix_b[8:4];

         alpha_blend_rgb444 = {out_r, out_g, out_b};
      end
   endfunction

   function automatic logic [4:0] fade_alpha_from_level(
      input logic [3:0] level
   );
      begin
         // Smaller level = closer to current sweep = more visible.
         // Larger level = older fade = more transparent.
         //
         // Scale: 0 to 16.

         case (level)
            4'd1: fade_alpha_from_level = 5'd13;
            4'd2: fade_alpha_from_level = 5'd11;
            4'd3: fade_alpha_from_level = 5'd9;
            4'd4: fade_alpha_from_level = 5'd7;
            4'd5: fade_alpha_from_level = 5'd5;
            4'd6: fade_alpha_from_level = 5'd4;
            4'd7: fade_alpha_from_level = 5'd3;
            4'd8: fade_alpha_from_level = 5'd2;
            default: fade_alpha_from_level = 5'd1;
         endcase
      end
   endfunction

   // ------------------------------------------------------------
   // Combinational geometry for current pixel
   // ------------------------------------------------------------

   logic signed [12:0] dx_c;
   logic signed [12:0] dy_c;

   logic signed [31:0] dx_sq_c;
   logic signed [31:0] dy_sq_c;
   logic signed [31:0] radius_sq_c;

   logic inside_circle_c;
   logic inside_mode_c;

   logic sweep_hit_c;
   logic object_hit_c;

   logic fade_hit_c;
   logic [4:0] fade_alpha_c;

   integer f;
   integer hist_index;

   always_comb begin
      // Convert VGA coordinates into radar-centered coordinates.
      //
      // dx positive = right of radar center
      // dy positive = above radar center
      //
      // dy uses y0 - y because VGA y increases downward.
      dx_c = $signed({1'b0, x})  - $signed({1'b0, x0});
      dy_c = $signed({1'b0, y0}) - $signed({1'b0, y});

      dx_sq_c = dx_c * dx_c;
      dy_sq_c = dy_c * dy_c;
      radius_sq_c = dx_sq_c + dy_sq_c;

      inside_circle_c = (radius_sq_c <= RADAR_RADIUS_SQ);

      // Mode 0: upper semicircle only.
      // Mode 1 and mode 2: full circle.
      if (mode == MODE_180)
         inside_mode_c = inside_circle_c && (dy_c >= 0);
      else
         inside_mode_c = inside_circle_c;

      // Current active sweep line.
      sweep_hit_c =
         inside_mode_c &&
         line_hit_test(
            dx_c,
            dy_c,
            cos_cur,
            sin_cur,
            8'd0,
            RADAR_RADIUS_U8,
            sweep_thickness
         );

      // Object beam.
      //
      // If distance = 30, object beam starts at radius 30
      // and extends to outer radius 200.
      object_hit_c =
         inside_mode_c &&
         object_detected &&
         line_hit_test(
            dx_c,
            dy_c,
            cos_cur,
            sin_cur,
            distance,
            RADAR_RADIUS_U8,
            object_thickness
         );

      // Fade trail.
      fade_hit_c   = 1'b0;
      fade_alpha_c = 5'd0;

      if (fade_enable && (fade_levels != 4'd0)) begin
         for (f = 1; f < HIST_DEPTH; f = f + 1) begin
            // fade_step controls spacing between history samples.
            //
            // fade_step = 1:
            //   fade levels use hist[1], hist[2], hist[3], ...
            //
            // fade_step = 2:
            //   fade levels use hist[2], hist[4], hist[6], ...
            //
            // fade_step = 0 is treated like fade_step = 1.
            if (fade_step == 4'd0)
               hist_index = f;
            else
               hist_index = f * fade_step;

            if ((f <= fade_levels) && (hist_index < HIST_DEPTH)) begin
               if (!fade_hit_c) begin
                  if (line_hit_test(
                         dx_c,
                         dy_c,
                         cos_hist[hist_index],
                         sin_hist[hist_index],
                         8'd0,
                         RADAR_RADIUS_U8,
                         fade_thickness
                      )) begin
                     fade_hit_c   = 1'b1;
                     fade_alpha_c = fade_alpha_from_level(f[3:0]);
                  end
               end
            end
         end
      end
   end

   // ------------------------------------------------------------
   // Two-stage VGA pixel delay pipeline
   // ------------------------------------------------------------
   //
   // Stage 1:
   //   latch current pixel RGB and overlay-hit decisions.
   //
   // Stage 2:
   //   delay those values one more cycle.
   //
   // Output:
   //   final mux uses stage-2 RGB and stage-2 hit decisions.
   //
   // This means the modified pixel stream is internally aligned
   // with the delayed RGB path.
   // ------------------------------------------------------------

   logic [11:0] rgb_d1;
   logic [11:0] rgb_d2;

   logic [11:0] color_d1;
   logic [11:0] color_d2;

   logic sweep_hit_d1;
   logic sweep_hit_d2;

   logic object_hit_d1;
   logic object_hit_d2;

   logic fade_hit_d1;
   logic fade_hit_d2;

   logic [4:0] fade_alpha_d1;
   logic [4:0] fade_alpha_d2;

   always_ff @(posedge clk, posedge reset) begin
      if (reset) begin
         rgb_d1         <= 12'h000;
         rgb_d2         <= 12'h000;

         color_d1       <= 12'h0F0;
         color_d2       <= 12'h0F0;

         sweep_hit_d1   <= 1'b0;
         sweep_hit_d2   <= 1'b0;

         object_hit_d1  <= 1'b0;
         object_hit_d2  <= 1'b0;

         fade_hit_d1    <= 1'b0;
         fade_hit_d2    <= 1'b0;

         fade_alpha_d1  <= 5'd0;
         fade_alpha_d2  <= 5'd0;

         so_rgb         <= 12'h000;
      end else begin
         // Stage 1
         rgb_d1         <= si_rgb;
         color_d1       <= color;

         sweep_hit_d1   <= sweep_hit_c;
         object_hit_d1  <= object_hit_c;
         fade_hit_d1    <= fade_hit_c;
         fade_alpha_d1  <= fade_alpha_c;

         // Stage 2
         rgb_d2         <= rgb_d1;
         color_d2       <= color_d1;

         sweep_hit_d2   <= sweep_hit_d1;
         object_hit_d2  <= object_hit_d1;
         fade_hit_d2    <= fade_hit_d1;
         fade_alpha_d2  <= fade_alpha_d1;

         // Final priority mux
         //
         // Priority:
         //   1. object detection red beam
         //   2. active sweep line
         //   3. fade trail
         //   4. original framebuffer pixel
         if (object_hit_d2) begin
            // Object detection color is fixed red and does not depend on color register.
            so_rgb <= OBJECT_RED;
         end else if (sweep_hit_d2) begin
            so_rgb <= color_d2;
         end else if (fade_hit_d2) begin
            so_rgb <= alpha_blend_rgb444(rgb_d2, color_d2, fade_alpha_d2);
         end else begin
            so_rgb <= rgb_d2;
         end
      end
   end

endmodule