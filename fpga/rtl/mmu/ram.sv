`timescale 1ns / 1ps

module ram #(
    parameter int ADDR_SIZE = 23,
    parameter int DATA_SIZE = 23,
    parameter int RAM_SIZE = 255
    )(
    input clk,
    input write_en,
    input [ADDR_SIZE:0] read_addr,
    input [ADDR_SIZE:0] write_addr,

    input [DATA_SIZE:0] data_in,
    output reg [DATA_SIZE:0] data_out,
    output reg [DATA_SIZE:0] write_data_out
    );

    reg [DATA_SIZE:0] ram [RAM_SIZE:0];

    always @(posedge clk) begin
        if (write_en)
            ram[write_addr] <= data_in;
        data_out <= ram[read_addr];
        write_data_out <= ram[write_addr];
    end
endmodule
