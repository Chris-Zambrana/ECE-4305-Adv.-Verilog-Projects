`timescale 1ns / 1ps

module fifo_tb(

    );
    
    // signal declarations
    localparam DATA_WIDTH = 8;
    localparam ADDR_WIDTH = 3;
    localparam T = 10; //clock period
    
    logic clk, reset;
    logic wr, rd;
    logic [DATA_WIDTH*2 -1: 0] w_data;
    logic [DATA_WIDTH - 1: 0] r_data;
    logic full, empty;
    
    // instantiate module under test
    top_asymmetric_fifo #(.DATA_WIDTH(DATA_WIDTH), .ADDR_WIDTH(ADDR_WIDTH)) uut (.*);
    
    // 10 ns clock running forever
    always
    begin
        clk = 1'b1;
        #(T / 2);
        clk = 1'b0;
        #(T / 2);
    end
    
    // reset for the first half cylce
    initial
    begin
        reset = 1'b1;
        rd = 1'b0;
        wr = 1'b0;
        @(negedge clk);
        reset = 1'b0;
    end
    
    //test vectors
    initial
    begin
        // ----------------EMPTY-----------------------
        // write
        @(negedge clk);
        w_data = 16'hE2F1;
        wr = 1'b1;     
        @(negedge clk);
        wr = 1'b0;
        
        // write
        repeat(1) @(negedge clk);
        w_data = 16'h7A4C;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        
        // write
        repeat(1) @(negedge clk);
        w_data = 16'hB3D9;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;             
        
        // write
        repeat(1) @(negedge clk);
        w_data = 16'h5E8A;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;        

        // ----------------FULL-----------------------

        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;  

        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;   

        // write
        repeat(1) @(negedge clk);
        w_data = 16'hC61F;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;        
        
        // ----------------FULL-----------------------

        // write (full so shouldn't write)
        repeat(1) @(negedge clk);
        w_data = 16'h2B47;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;           
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // read
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;
        
        // ----------------EMPTY-----------------------
        
        // read & write at the same time 
        //(simultaneous rd & wr, so write should occur twice and read should read 1st value written)
        repeat(1) @(negedge clk);
        w_data = 16'h81B5;
        wr = 1'b1;
        rd = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        rd = 1'b0;
        
        // read upper most value written
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;

        // read while empty
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;

        // read while empty
        repeat(1) @(negedge clk);
        rd = 1'b1;
        @(negedge clk)
        rd = 1'b0;

        // ----------------NOT EMPTY-----------------------
        repeat(1) @(negedge clk);
        w_data = 16'hF2C8;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        
        repeat(1) @(negedge clk);
        w_data = 16'h3A96;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        
        repeat(1) @(negedge clk);
        w_data = 16'hAD4B;
        wr = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        
        // read & write at the same time 
        /*(full bc we are writing 2 values at a time and write pointer can't 
        //be one position before read pointer bc then read will read what is 
        //written in upper half of the input data we just wrote, which is supposed 
        to be the last value written, so write should not occur and read should 
        read 1st value written)*/
        repeat(1) @(negedge clk);
        w_data = 16'h6F17;
        wr = 1'b1;
        rd = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        rd = 1'b0;
        
        // read & write at the same time (should read only bc buffer is full)
        repeat(1) @(negedge clk);
        w_data = 16'h9DAB;
        wr = 1'b1;
        rd = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        rd = 1'b0;

        // read & write at the same time (should do both read and write)
        repeat(1) @(negedge clk);
        w_data = 16'hC53E;
        wr = 1'b1;
        rd = 1'b1;
        @(negedge clk)
        wr = 1'b0;
        rd = 1'b0;

        repeat(3) @(negedge clk);
        $stop;
                
        
    end
    
endmodule
