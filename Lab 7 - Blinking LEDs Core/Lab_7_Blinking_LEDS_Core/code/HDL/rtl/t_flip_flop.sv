module t_flip_flop(
    input  logic clk,
    input  logic reset,
    input  logic t,      // tick pulse
    output logic out
);
    always_ff @(posedge clk, posedge reset) begin
        if (reset)
            out <= 1'b0;
        else if (t)
            out <= ~out;     // TOGGLE on tick
        else
            out <= out;      // hold
    end
endmodule
