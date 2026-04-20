`timescale 1ns / 1ps

module fsm_early_debouncer(
        input logic clk, // genrally the 100MHz
        input logic reset,
        input logic sw,
        input logic tick,
        output logic db
    );
    
    typedef enum logic [2:0] {zero = 3'd0, wait1_1 = 3'd1, wait1_2 = 3'd2, wait1_3 = 3'd3, one = 3'd4, wait0_1 = 3'd5, wait0_2 = 3'd6, wait0_3 = 3'd7} state_type;
    
    state_type state_reg, state_next;
    
    always_ff @(posedge clk) begin
        if(reset) begin
            state_reg <= zero;
        end else begin
            state_reg <= state_next;
        end
    end
    
    always_comb begin
        state_next = state_reg;
        case (state_reg) 
            zero: begin
                if(sw) begin 
                    state_next = wait1_1;
                    db = 1;
                end else begin
                    state_next = zero;
                    db = 0;
                end
            end
            
            wait1_1: begin
                db = 1;
                if(tick) begin
                    state_next = wait1_2;
                end else begin
                    state_next = wait1_1;
                end 
            end
            
            wait1_2: begin
                db = 1;
                if(tick) begin
                    state_next = wait1_3;
                end else begin
                    state_next = wait1_2;
                end 
            end
            
            wait1_3: begin
                db = 1;
                if(tick) begin
                    state_next = one;
                end else begin
                    state_next = wait1_3;
                end 
            end
            
            one: begin
                if(!sw) begin 
                    state_next = wait0_1;
                    db = 0;
                end else begin
                    state_next = one;
                    db = 1;
                end
            end
            
            wait0_1: begin
                db = 0;
                if(tick) begin
                    state_next = wait0_2;
                end else begin
                    state_next = wait0_1;
                end 
            end
            
            wait0_2: begin
                db = 0;
                if(tick) begin
                    state_next = wait0_3;
                    db = 0;
                end else begin
                    state_next = wait0_2;
                    db = 0;
                end 
            end
            
            wait0_3: begin
                db = 0;
                if(tick) begin
                    state_next = zero;
                end else begin
                    state_next = wait0_3;
                end 
            end
            
            default: begin 
                state_next = zero;
            end
        endcase
    end
        
    endmodule
