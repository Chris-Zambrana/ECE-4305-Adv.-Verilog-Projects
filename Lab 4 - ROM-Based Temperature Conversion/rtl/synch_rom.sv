module synch_rom
#(
    parameter DATA_BITS = 8,
    parameter ADDRESS_BITS = 9
)
(
    input logic clk,
    input logic [ADDRESS_BITS-1:0] addr,
    output logic [DATA_BITS-1:0] data
);
    
    // signal declaration
    (*rom_style = "block" *)logic [DATA_BITS-1:0] rom [0:(2**ADDRESS_BITS)-1];
    
    initial
        $readmemh("conversion_table.mem", rom);
        
    always_ff @(posedge clk)
        data <= rom[addr];
endmodule