module radar_angle_lut (
   input  logic [8:0] angle_deg,
   output logic signed [9:0] cos_q8,
   output logic signed [9:0] sin_q8
);

   logic [8:0] angle_norm;
   logic [8:0] quad_deg_9;
   logic [6:0] quad_deg;

   function automatic logic signed [9:0] sin_0_to_90_q8(
      input logic [6:0] deg
   );
      begin
         case (deg)
            7'd0:  sin_0_to_90_q8 = 10'sd0;
            7'd1:  sin_0_to_90_q8 = 10'sd4;
            7'd2:  sin_0_to_90_q8 = 10'sd9;
            7'd3:  sin_0_to_90_q8 = 10'sd13;
            7'd4:  sin_0_to_90_q8 = 10'sd18;
            7'd5:  sin_0_to_90_q8 = 10'sd22;
            7'd6:  sin_0_to_90_q8 = 10'sd27;
            7'd7:  sin_0_to_90_q8 = 10'sd31;
            7'd8:  sin_0_to_90_q8 = 10'sd36;
            7'd9:  sin_0_to_90_q8 = 10'sd40;

            7'd10: sin_0_to_90_q8 = 10'sd44;
            7'd11: sin_0_to_90_q8 = 10'sd49;
            7'd12: sin_0_to_90_q8 = 10'sd53;
            7'd13: sin_0_to_90_q8 = 10'sd58;
            7'd14: sin_0_to_90_q8 = 10'sd62;
            7'd15: sin_0_to_90_q8 = 10'sd66;
            7'd16: sin_0_to_90_q8 = 10'sd71;
            7'd17: sin_0_to_90_q8 = 10'sd75;
            7'd18: sin_0_to_90_q8 = 10'sd79;
            7'd19: sin_0_to_90_q8 = 10'sd83;

            7'd20: sin_0_to_90_q8 = 10'sd88;
            7'd21: sin_0_to_90_q8 = 10'sd92;
            7'd22: sin_0_to_90_q8 = 10'sd96;
            7'd23: sin_0_to_90_q8 = 10'sd100;
            7'd24: sin_0_to_90_q8 = 10'sd104;
            7'd25: sin_0_to_90_q8 = 10'sd108;
            7'd26: sin_0_to_90_q8 = 10'sd112;
            7'd27: sin_0_to_90_q8 = 10'sd116;
            7'd28: sin_0_to_90_q8 = 10'sd120;
            7'd29: sin_0_to_90_q8 = 10'sd124;

            7'd30: sin_0_to_90_q8 = 10'sd128;
            7'd31: sin_0_to_90_q8 = 10'sd132;
            7'd32: sin_0_to_90_q8 = 10'sd136;
            7'd33: sin_0_to_90_q8 = 10'sd139;
            7'd34: sin_0_to_90_q8 = 10'sd143;
            7'd35: sin_0_to_90_q8 = 10'sd147;
            7'd36: sin_0_to_90_q8 = 10'sd150;
            7'd37: sin_0_to_90_q8 = 10'sd154;
            7'd38: sin_0_to_90_q8 = 10'sd158;
            7'd39: sin_0_to_90_q8 = 10'sd161;

            7'd40: sin_0_to_90_q8 = 10'sd165;
            7'd41: sin_0_to_90_q8 = 10'sd168;
            7'd42: sin_0_to_90_q8 = 10'sd171;
            7'd43: sin_0_to_90_q8 = 10'sd175;
            7'd44: sin_0_to_90_q8 = 10'sd178;
            7'd45: sin_0_to_90_q8 = 10'sd181;
            7'd46: sin_0_to_90_q8 = 10'sd184;
            7'd47: sin_0_to_90_q8 = 10'sd187;
            7'd48: sin_0_to_90_q8 = 10'sd190;
            7'd49: sin_0_to_90_q8 = 10'sd193;

            7'd50: sin_0_to_90_q8 = 10'sd196;
            7'd51: sin_0_to_90_q8 = 10'sd199;
            7'd52: sin_0_to_90_q8 = 10'sd202;
            7'd53: sin_0_to_90_q8 = 10'sd204;
            7'd54: sin_0_to_90_q8 = 10'sd207;
            7'd55: sin_0_to_90_q8 = 10'sd210;
            7'd56: sin_0_to_90_q8 = 10'sd212;
            7'd57: sin_0_to_90_q8 = 10'sd215;
            7'd58: sin_0_to_90_q8 = 10'sd217;
            7'd59: sin_0_to_90_q8 = 10'sd219;

            7'd60: sin_0_to_90_q8 = 10'sd222;
            7'd61: sin_0_to_90_q8 = 10'sd224;
            7'd62: sin_0_to_90_q8 = 10'sd226;
            7'd63: sin_0_to_90_q8 = 10'sd228;
            7'd64: sin_0_to_90_q8 = 10'sd230;
            7'd65: sin_0_to_90_q8 = 10'sd232;
            7'd66: sin_0_to_90_q8 = 10'sd234;
            7'd67: sin_0_to_90_q8 = 10'sd236;
            7'd68: sin_0_to_90_q8 = 10'sd237;
            7'd69: sin_0_to_90_q8 = 10'sd239;

            7'd70: sin_0_to_90_q8 = 10'sd241;
            7'd71: sin_0_to_90_q8 = 10'sd242;
            7'd72: sin_0_to_90_q8 = 10'sd243;
            7'd73: sin_0_to_90_q8 = 10'sd245;
            7'd74: sin_0_to_90_q8 = 10'sd246;
            7'd75: sin_0_to_90_q8 = 10'sd247;
            7'd76: sin_0_to_90_q8 = 10'sd248;
            7'd77: sin_0_to_90_q8 = 10'sd249;
            7'd78: sin_0_to_90_q8 = 10'sd250;
            7'd79: sin_0_to_90_q8 = 10'sd251;

            7'd80: sin_0_to_90_q8 = 10'sd252;
            7'd81: sin_0_to_90_q8 = 10'sd253;
            7'd82: sin_0_to_90_q8 = 10'sd254;
            7'd83: sin_0_to_90_q8 = 10'sd254;
            7'd84: sin_0_to_90_q8 = 10'sd255;
            7'd85: sin_0_to_90_q8 = 10'sd255;
            7'd86: sin_0_to_90_q8 = 10'sd255;
            7'd87: sin_0_to_90_q8 = 10'sd256;
            7'd88: sin_0_to_90_q8 = 10'sd256;
            7'd89: sin_0_to_90_q8 = 10'sd256;
            7'd90: sin_0_to_90_q8 = 10'sd256;

            default: sin_0_to_90_q8 = 10'sd0;
         endcase
      end
   endfunction

   always_comb begin
      if (angle_deg >= 9'd360)
         angle_norm = angle_deg - 9'd360;
      else
         angle_norm = angle_deg;

      quad_deg_9 = 9'd0;
      quad_deg   = 7'd0;
      sin_q8     = 10'sd0;
      cos_q8     = 10'sd256;

      if (angle_norm <= 9'd90) begin
         // Quadrant I: 0 to 90 degrees
         quad_deg_9 = angle_norm;
         quad_deg   = quad_deg_9[6:0];

         sin_q8 = sin_0_to_90_q8(quad_deg);
         cos_q8 = sin_0_to_90_q8(7'd90 - quad_deg);

      end else if (angle_norm <= 9'd180) begin
         // Quadrant II: 91 to 180 degrees
         quad_deg_9 = 9'd180 - angle_norm;
         quad_deg   = quad_deg_9[6:0];

         sin_q8 = sin_0_to_90_q8(quad_deg);
         cos_q8 = -sin_0_to_90_q8(7'd90 - quad_deg);

      end else if (angle_norm <= 9'd270) begin
         // Quadrant III: 181 to 270 degrees
         quad_deg_9 = angle_norm - 9'd180;
         quad_deg   = quad_deg_9[6:0];

         sin_q8 = -sin_0_to_90_q8(quad_deg);
         cos_q8 = -sin_0_to_90_q8(7'd90 - quad_deg);

      end else begin
         // Quadrant IV: 271 to 359 degrees
         quad_deg_9 = 9'd360 - angle_norm;
         quad_deg   = quad_deg_9[6:0];

         sin_q8 = -sin_0_to_90_q8(quad_deg);
         cos_q8 = sin_0_to_90_q8(7'd90 - quad_deg);
      end
   end

endmodule