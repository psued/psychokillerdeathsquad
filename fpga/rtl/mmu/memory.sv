`timescale 1ns / 1ps

localparam int WIDTHL = 0;
localparam int WIDTHH = WIDTHL + 15;
localparam int HEIGHTL = WIDTHH + 1;
localparam int HEIGHTH = HEIGHTL + 15;
localparam int SRCXL = HEIGHTH + 1;
localparam int SRCXH = SRCXL + 15;
localparam int SRCYL = SRCXH + 1;
localparam int SRCYH = SRCYL + 15;
localparam int DSTXL = SRCYH + 1;
localparam int DSTXH = DSTXL + 15;
localparam int DSTYL = DSTXH + 1;
localparam int DSTYH = DSTYL + 15;
localparam int ZL = DSTYH + 1;
localparam int ZH = ZL + 7;
localparam int ENABLED = ZH + 1;
localparam int FLIPPED = ENABLED + 1;
localparam int COLORBITSL = FLIPPED + 1;
localparam int COLORBITSH = COLORBITSL + 7;
localparam int BUSY = COLORBITSH + 1;
localparam int FILENAMEL = BUSY + 1;
localparam int FILENAMEH = FILENAMEL + 15;
localparam int ADDRL = FILENAMEH + 1;
localparam int ADDRH = ADDRL + 7;
localparam int LAYERSIZE = ADDRH;
localparam int NUMLAYERS = 12;

module memory(
    input clk,
    input rst,

    input [4:0] layer,
    output [15:0] width,
    output [15:0] height,
    output [15:0] src_x,
    output [15:0] src_y,
    output [15:0] dst_x,
    output [15:0] dst_y,
    output [7:0] z,
    output enabled,
    output flipped,
    output [7:0] bits_per_color,

    input [7:0] addr, // relative to layer
    output [31:0] data,

    output busy,
    output [15:0] file_name
    );

    reg [LAYERSIZE:0] layers [NUMLAYERS];

    assign width = layers[layer][WIDTHH:WIDTHL];
    assign height = layers[layer][HEIGHTH:HEIGHTL];
    assign src_x = layers[layer][SRCXH:SRCXL];
    assign src_y = layers[layer][SRCYH:SRCYL];
    assign dst_x = layers[layer][DSTXH:DSTXL];
    assign dst_y = layers[layer][DSTYH:DSTYL];
    assign z = layers[layer][ZH:ZL];
    assign enabled = layers[layer][ENABLED];
    assign flipped = layers[layer][FLIPPED];
    assign bits_per_color = layers[layer][COLORBITSH:COLORBITSL];
    assign busy = layers[layer][BUSY];

    wire [15:0] file_name = layers[layer][FILENAMEH:FILENAMEL];
    wire [7:0] address = layers[layer][ADDRH:ADDRL] + addr; // addresses should definitely be bigger


    reg [7:0] write_addr;
    reg [7:0] write_data;

    ram d(
        .clk(clk),
        .write_en(write_en),
        .read_addr(address),
        .write_addr(write_addr),

        .data_in(write_data),
        .data_out(data)
    );

    always @ (posedge rst, posedge clk) begin
        if (rst) begin
            for (i = 0; i < NUMLAYERS; i = i + 1)
                layers[i] <= 0;

            // set start address of each layer
            layers[1][ADDRH:ADDRL] <= 4;
        end else begin

        end
    end

endmodule
