module tb_blinking_led ();
    localparam W = 16;  // width of output port
    localparam CLK_PERIOD = 10; // clock period is 10 ns for 100 MHz clock

    logic clk;
    logic reset;
    // slot interface
    logic cs;
    logic read; // not used
    logic write;
    logic [4:0] addr;
    logic [31:0] wr_data;
    logic [31:0] rd_data; // not used
    // external output  
    logic [W-1:0] dout;

    // instantiate the blinking LED module
    chu_blinking_led #(.W(W)) dut (
        .clk(clk),
        .reset(reset),
        .cs(cs),
        .read(read),
        .write(write),
        .addr(addr),
        .wr_data(wr_data),
        .rd_data(rd_data),
        .dout(dout)
    );

    // clock generation
    always #(CLK_PERIOD/2) clk = ~clk;

    // LED monitoring variables
    logic [3:0] led_prev;
    longint led_last_on_time [3:0];
    longint led_on_start [3:0];
    longint led_on_duration [3:0];
    longint led_off_duration [3:0];
    longint led_cycle_duration [3:0];
    
    // LED monitoring - track transitions
    always @(posedge clk) begin
        for (int i = 0; i < 4; i = i + 1) begin
            // Detect rising edge (LED turning on)
            if (dut.dout[i] && !led_prev[i]) begin
                led_on_start[i] = $time;
                // Calculate cycle duration (from last ON to this ON)
                if (led_last_on_time[i] != 0) begin
                    led_cycle_duration[i] = $time - led_last_on_time[i];
                    $display("[%0t ns] LED%0d: ON again (cycle duration: %0d ns = %.3f ms = %.3f seconds)", 
                             $time, i, led_cycle_duration[i], real'(led_cycle_duration[i]) / 1_000_000.0,
                             real'(led_cycle_duration[i]) / 1_000_000_000.0);
                end else begin
                    $display("[%0t ns] LED%0d: ON (first)", $time, i);
                end
                led_last_on_time[i] = $time;
            end
            // Detect falling edge (LED turning off)
            else if (!dut.dout[i] && led_prev[i]) begin
                led_on_duration[i] = $time - led_on_start[i];
                $display("[%0t ns] LED%0d: OFF (ON was: %0d ns = %.3f ms)", 
                         $time, i, led_on_duration[i], real'(led_on_duration[i]) / 1_000_000.0);
            end
        end
        led_prev <= dut.dout[3:0];
    end

    // test sequence
    // To be slow enough for human observation, lowest value to write into is 50 (50ms) 
    initial begin
        clk = 0;
        reset = 1;
        cs = 0;
        write = 0;
        addr = 0;
        wr_data = 0;
        led_prev = 4'b0;

        @(posedge clk);
        reset = 0; // release reset
        cs = 1; // enable chip select
        write = 1; // enable write
        addr = 0; // select reg0
        wr_data = 32'h00000001; // set reg0 for timer0 to 1 ms (65 seconds)

        @(posedge clk);
        addr = 1; // select reg1
        wr_data = 32'h00000002; // set reg1 for timer1 to 2 ms (34 seconds)

        @(posedge clk);
        addr = 2; // select reg2
        wr_data = 32'h00000003; // set reg2 for timer2 to 3 ms (20 seconds)
        
        @(posedge clk);
        addr = 3; // select reg3    
        wr_data = 32'h00000004; // set reg3 for timer3 to 4 ms (10 seconds)

    end
endmodule
