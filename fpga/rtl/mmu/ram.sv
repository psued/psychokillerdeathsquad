`timescale 1ns / 1ps

localparam int ADDR_SIZE = 7;
localparam int DATA_SIZE = 31;
localparam int RAM_SIZE = 255;

// based on UG901 v2022.2 page 120
module ram(
    input clk,
    input write_en,
    input [ADDR_SIZE:0] read_addr,
    input [ADDR_SIZE:0] write_addr,

    input [DATA_SIZE:0] data_in,
    output [DATA_SIZE:0] data_out
    );

    reg [DATA_SIZE:0] ram [RAM_SIZE:0];

    always @(posedge clk) begin
        if (write_en)
            ram[write_addr] <= data_in;
    end

    assign data_out = ram[read_addr];
endmodule
