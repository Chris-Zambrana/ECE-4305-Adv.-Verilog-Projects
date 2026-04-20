`timescale 1ns / 1ps

// Questions to Ask
// 1. Does the counter need to db change immediately when sw changes or can it change in the next cylce 
//(will have to include a db signal in the fsm and that'll be the output for the circuit)
// 2. Can you use only 1 state to control the 20ms delay? (Makes the whole start situation much easier)

module top_early_debouncer
#(
    parameter MAX_COUNT = 1000000
)
(
        input logic clk, // genrally the 100MHz
        input logic reset,
        input logic sw,
        output logic db,
        output logic [1:0] JA
);

    logic tick;    

    assign JA[0] = sw;
    assign JA[1] = db;
    
    counter_early_debouncer #(MAX_COUNT) counter0 (clk,reset,tick);
    
    fsm_early_debouncer fsm0(clk,reset,sw,tick,db);
    
endmodule
