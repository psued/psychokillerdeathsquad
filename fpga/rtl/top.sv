module top(
    input  logic       clk,
    input  logic       reset,

    output logic       hsync,
    output logic       vsync,
    output logic [3:0] red,
    output logic [3:0] green,
    output logic [3:0] blue

);

    // ============================================================
    // Clock generation
    // ============================================================

    logic vga_clk;
    logic clk_locked;

    clk_wiz_vga clock_generator (
        .clk_in1  (clk),          // 100 MHz Arty oscillator
        .reset    (reset),        // BTN0, active high
        .clk_out1 (vga_clk),      // generated 25 MHz clock
        .locked   (clk_locked)
    );

    logic system_reset;
    logic vga_timing_reset;
    logic vga_started;

    assign system_reset = reset | ~clk_locked;
    assign vga_timing_reset = system_reset | ~vga_started;

    logic hsync_raw;
    logic vsync_raw;
    logic video_on_raw;
    logic [9:0] x;
    logic [9:0] y;

    vga_sync vga_sync_inst (
        .clk(vga_clk),
        .reset(vga_timing_reset),
        .hsync(hsync_raw),
        .vsync(vsync_raw),
        .video_on(video_on_raw),
        .x(x),
        .y(y)
    );

    logic [2:0] produced_toggle;
    logic [2:0] produced_sync;

    cdc_sync produced_sync_inst (
        .clk(vga_clk),
        .async_in(produced_toggle),
        .sync_out(produced_sync)
    );

    logic [2:0] consumed_toggle;
    logic [2:0] consumed_sync;

    cdc_sync consumed_sync_inst (
        .clk(clk),
        .async_in(consumed_toggle),
        .sync_out(consumed_sync)
    );

    logic [1:0] write_buffer;
    logic [9:0] write_x;
    logic [9:0] render_x;
    logic write_enable;
    logic [11:0] pixel_in;
    logic stall;

    line_buffer_write_controller write_controller (
        .clk(clk),
        .reset(system_reset),

        .render_x(render_x),

        .consumed_sync(consumed_sync),
        .produced_toggle(produced_toggle),

        .write_buffer(write_buffer),
        .stall(stall)
    );

    mock_renderer mock_renderer_inst (
        .clk(clk),
        .reset(system_reset),
        .stall(stall),

        .write_x(write_x),
        .render_x(render_x),
        .write_enable(write_enable),
        .pixel_in(pixel_in)
    );

    logic buffer_ready;
    logic [1:0] read_buffer;

    line_buffer_read_controller read_controller (
        .clk(vga_clk),
        .reset(system_reset),
        .x(x),
        .video_on_raw(video_on_raw),

        .produced_sync(produced_sync),
        .consumed_toggle(consumed_toggle),

        .buffer_ready(buffer_ready),
        .read_buffer(read_buffer),
        .vga_started(vga_started)
    );

    logic read_enable;
    logic [11:0] pixel_out;

    line_buffer_reader line_buffer_reader_inst (
        .clk(vga_clk),
        .reset(system_reset),

        .pixel_out(pixel_out),
        .buffer_ready(buffer_ready),

        .hsync_raw(hsync_raw),
        .vsync_raw(vsync_raw),
        .video_on_raw(video_on_raw),

        .read_enable(read_enable),
        .hsync(hsync),
        .vsync(vsync),
        .red(red),
        .green(green),
        .blue(blue)
    );

    line_buffer_bram line_buffer (
        .write_buffer(write_buffer),
        .write_x(write_x),
        .write_enable(write_enable),
        .write_clk(clk),
        .pixel_in(pixel_in),

        .read_buffer(read_buffer),
        .read_x(x),
        .read_enable(read_enable),
        .read_clk(vga_clk),
        .pixel_out(pixel_out)
    );



endmodule
