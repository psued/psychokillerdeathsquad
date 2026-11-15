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
localparam int ASSETWIDTHL = COLORBITSH + 1;
localparam int ASSETWIDTHH = ASSETWIDTHL + 15;
localparam int FILENAMEL = ASSETWIDTHH + 1;
localparam int FILENAMEH = FILENAMEL + 15;
localparam int FILENAMEPREVL = FILENAMEH + 1;
localparam int FILENAMEPREVH = FILENAMEPREVL + 15;
localparam int ADDRL = FILENAMEPREVH + 1;
localparam int ADDRH = ADDRL + 7;
localparam int MEMSIZEL = ADDRH + 1;
localparam int MEMSIZEH = MEMSIZEL + 15;
localparam int LAYERSIZE = MEMSIZEH;
localparam int NUMLAYERS = 24;

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
    output [15:0] asset_width,

    input [15:0] addr, // relative to layer
    output [23:0] data,

    output [15:0] file_name,
    output [15:0] file_name_prev,
    output [15:0] mem_size,

    input write_en,
    input [15:0] write_addr, // relative to layer
    input [23:0] write_data,
    output [23:0] read_write_data,
    input write_mcu,
    input [4:0] layer_write,
    input [7:0] mcu_data,
    input mcu_byte_done,
    input [7:0] set_color_bits,
    input [15:0] set_asset_width
    );

    reg mcu_byte_done_prev;
    wire pos_mcu_byte_done = mcu_byte_done && !mcu_byte_done_prev;
    reg [3:0] mcu_counter;

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
    assign asset_width = layers[layer][ASSETWIDTHH:ASSETWIDTHL];

    // file name is only used by the writer
    assign file_name = layers[layer_write][FILENAMEH:FILENAMEL];
    assign file_name_prev = layers[layer_write][FILENAMEPREVH:FILENAMEPREVL];
    assign mem_size = layers[layer_write][MEMSIZEH:MEMSIZEL];

    wire [23:0] address = layers[layer][ADDRH:ADDRL] + addr;
    wire [23:0] write_address = layers[layer_write][ADDRH:ADDRL] + write_addr;

    ram d(
        .clk(clk),
        .write_en(write_en),
        .read_addr(address),
        .write_addr(write_address),

        .data_in(write_data),
        .data_out(data),
        .write_data_out(read_write_data)
    );

    integer i;

    always @ (posedge rst, posedge clk) begin
        if (rst) begin
            mcu_byte_done_prev <= 0;
            mcu_counter <= 0;

            for (i = 0; i < NUMLAYERS; i = i + 1)
                layers[i] <= 0;

            // set start address of each layer and layer size
            // layers[0] has size 0 since the ram is separate to save space
            // e.g. layers[1][ADDRH:ADDRL] <= 0; layers[1][MEMSIZEH:MEMSIZEL] <= 4;
        end else begin
            mcu_byte_done_prev <= mcu_byte_done;

            if (write_en) begin
                layers[layer_write][COLORBITSH:COLORBITSL] <= set_color_bits;
                layers[layer_write][ASSETWIDTHH:ASSETWIDTHL] <= set_asset_width;
            end

            if (write_mcu && pos_mcu_byte_done) begin
                mcu_counter <= mcu_counter + 1;
                case (mcu_counter)
                    0: begin
                        layers[layer_write][FILENAMEL+7:FILENAMEL] <= mcu_data;
                        layers[layer_write][FILENAMEPREVH:FILENAMEPREVL] <= layers[layer_write][FILENAMEH:FILENAMEL];
                    end
                    1: layers[layer_write][FILENAMEH:FILENAMEH-7] <= mcu_data;
                    2: layers[layer_write][SRCXL+7:SRCXL] <= mcu_data;
                    3: layers[layer_write][SRCXH:SRCXH-7] <= mcu_data;
                    4: layers[layer_write][SRCYL+7:SRCYL] <= mcu_data;
                    5: layers[layer_write][SRCYH:SRCYH-7] <= mcu_data;
                    6: layers[layer_write][WIDTHL+7:WIDTHL] <= mcu_data;
                    7: layers[layer_write][WIDTHH:WIDTHH-7] <= mcu_data;
                    8: layers[layer_write][HEIGHTL+7:HEIGHTL] <= mcu_data;
                    9: layers[layer_write][HEIGHTH:HEIGHTH-7] <= mcu_data;
                    10: layers[layer_write][DSTXL+7:DSTXL] <= mcu_data;
                    11: layers[layer_write][DSTXH:DSTXH-7] <= mcu_data;
                    12: layers[layer_write][DSTYL+7:DSTYL] <= mcu_data;
                    13: layers[layer_write][DSTYH:DSTYH-7] <= mcu_data;
                    14: layers[layer_write][ZH:ZL] <= mcu_data;
                    15: begin // flags
                        layers[layer_write][ENABLED] <= mcu_data[0];
                        layers[layer_write][FLIPPED] <= mcu_data[1];
                    end
                endcase
            end
        end
    end
endmodule
