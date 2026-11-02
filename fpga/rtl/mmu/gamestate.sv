`timescale 1ns / 1ps

module gamestate(
    input clk,
    input rst,

    // begin: for renderer
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
    output busy,

    input [7:0] addr, // relative to layer
    output [31:0] data,
    // end: for renderer

    output SD_SCLK,
    output SD_DI,
    input SD_DO,
    output SD_CS,

    input MCU_SCLK,
    output MCU_MISO,
    input MCU_MOSI,
    input MCU_CS
    );

    wire sd_init_done;
    reg [31:0] sd_addr;
    reg [31:0] sd_size; // (fat32_size - 1) >> 9
    reg sd_read;
    wire sd_reading;
    wire sd_byte_ready;
    wire [7:0] sd_data;

    sdcard sd(
        .clk(clk),
        .rst(rst),

        .SCLK(SD_SCLK),
        .DI(SD_DI),
        .DO(SD_DO),
        .CS(SD_CS),

        // .error(sd_error),
        .init_done(sd_init_done),

        .addr(sd_addr),
        .size(sd_size),
        .read(sd_read),
        .reading(sd_reading),
        .byte_ready(sd_byte_ready),
        .data(sd_data)
    );

    // TODO: clock synchronize
    wire [7:0] mcu_data;
    wire mcu_byte_done;
    reg mcu_byte_done_prev;

    spi mcu(
        .rst(rst),

        .SCLK(MCU_SCLK),
        .DOUT(MCU_MISO),
        .DIN(MCU_MOSI),
        .CS(MCU_CS),

        .data_recv(mcu_data),
        .data_received(mcu_byte_done)
    );
endmodule
