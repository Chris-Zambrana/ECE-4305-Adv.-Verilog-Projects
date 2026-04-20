`timescale 1ns / 1ps

module tb_square_wave_gen;

  localparam int  WIDTH       = 4;
  localparam int  NEW_WIDTH = 8;
  localparam time CLK_PER     = 10ns;

  logic clk;
  logic reset;
  logic [WIDTH-1:0] m, n;
  logic out;

  // DUT
  top_square_wave_gen #(
    .WIDTH(WIDTH),
    .NEW_WIDTH(NEW_WIDTH)
  ) dut (
    .clk   (clk),
    .reset (reset),
    .m     (m),
    .n     (n),
    .out   (out)
  );

  // 100 MHz clock
  initial clk = 1'b0;
  always #(CLK_PER/2) clk = ~clk;

  // Print whenever out toggles
  logic out_prev;
  initial out_prev = 1'b0;
  always @(posedge clk) begin
    out_prev <= out;
    if (!reset && (out_prev !== out)) begin
      $display("[%0t] out toggled to %0b (m=%0d, n=%0d)", $time, out, m, n);
    end
  end

  initial begin
    // ---- start at 0ns with known values ----
    reset = 1'b1;
    m     = 4'd3;
    n     = 4'd2;

    // hold reset for a few cycles
    #15 reset = 1'b0;

    #2000;

    @(posedge clk);
    m = 4'd5;
    n = 4'd9;

    #6000;

    $finish;
  end

endmodule
