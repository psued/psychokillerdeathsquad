module top #(
    parameter int PIXELS_PER_CYCLE = 2,

    localparam int LINE_PIXELS = 320,
    localparam int FRAME_LINES = 240,
    localparam int CHANNEL_BITS = 4,
    localparam int PIXEL_SLOT_BITS = 16,
    localparam int BUFFER_COUNT = 2,
    localparam int HORIZONTAL_SCALE = 2,

    localparam int PIXEL_BITS = 3 * CHANNEL_BITS,
    localparam int X_WIDTH = $clog2(LINE_PIXELS),
    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE
)(
    input  logic       clk,
    input  logic       reset,

    output logic       hsync,
    output logic       vsync,
    output logic [CHANNEL_BITS-1:0] red,
    output logic [CHANNEL_BITS-1:0] green,
    output logic [CHANNEL_BITS-1:0] blue

);

    // ============================================================
    // Clock generation
    // ============================================================

    logic clk_input;
    logic system_clk;
    logic vga_clk;
    logic clk_locked;

    // Share one globally buffered 100 MHz clock across both consumers.
    IBUF system_clock_input (
        .I(clk),
        .O(clk_input)
    );

    BUFG system_clock_buffer (
        .I(clk_input),
        .O(system_clk)
    );

    clk_wiz_vga clock_generator (
        .clk_in1  (system_clk),   // Buffered 100 MHz board oscillator.
        .reset    (reset),        // Active-high user reset.
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
    logic [9:0] x; // VGA raster: 0–799, including blanking.
    logic [9:0] y; // VGA raster: 0–524, including blanking.

    vga_sync vga_sync_inst (
        .clk(vga_clk),
        .reset(vga_timing_reset),
        .hsync(hsync_raw),
        .vsync(vsync_raw),
        .video_on(video_on_raw),
        .x(x),
        .y(y)
    );

    logic [BUFFER_COUNT-1:0] produced_toggle;
    logic [BUFFER_COUNT-1:0] produced_sync;

    cdc_sync produced_sync_inst (
        .clk(vga_clk),
        .async_in(produced_toggle),
        .sync_out(produced_sync)
    );

    logic [BUFFER_COUNT-1:0] consumed_toggle;
    logic [BUFFER_COUNT-1:0] consumed_sync;

    cdc_sync consumed_sync_inst (
        .clk(system_clk),
        .async_in(consumed_toggle),
        .sync_out(consumed_sync)
    );

    logic write_buffer;
    logic [X_WIDTH-1:0] write_x;
    logic line_done;
    logic write_enable;
    logic [WORD_WIDTH-1:0] pixel_in;
    logic [PIXELS_PER_CYCLE-1:0] write_mask;
    logic stall;

    renderer_pkg::layer_descriptor_t snapshot [0:renderer_pkg::LAYER_COUNT-1];
    logic [renderer_pkg::IMAGE_ID_WIDTH-1:0] mmu_request_image_id;
    logic [15:0] mmu_request_x [0:PIXELS_PER_CYCLE-1];
    logic [15:0] mmu_request_y [0:PIXELS_PER_CYCLE-1];
    logic mmu_request_valid;
    logic [PIXEL_BITS-1:0] mmu_pixel_output [0:PIXELS_PER_CYCLE-1];

    line_buffer_write_controller write_controller (
        .clk(system_clk),
        .reset(system_reset),

        .line_done(line_done),

        .consumed_sync(consumed_sync),
        .produced_toggle(produced_toggle),

        .write_buffer(write_buffer),
        .stall(stall)
    );

    mock_snapshot snapshot_source (
        .snapshot(snapshot)
    );

    renderer #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) renderer_inst (
        .clk(system_clk),
        .reset(system_reset),
        .stall(stall),
        .snapshot(snapshot),

        .mmu_request_image_id(mmu_request_image_id),
        .mmu_request_x(mmu_request_x),
        .mmu_request_y(mmu_request_y),
        .mmu_request_valid(mmu_request_valid),
        .mmu_pixel_output(mmu_pixel_output),

        .bram_write_x(write_x),
        .bram_write_enable(write_enable),
        .bram_pixel_in(pixel_in),
        .bram_write_mask(write_mask),
        .line_done(line_done),
        .rendering()
    );

    mock_mmu #(
        .LATENCY(3),
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) mock_mmu_inst (
        .clk(system_clk),
        .reset(system_reset),
        .x(mmu_request_x),
        .y(mmu_request_y),
        .image_id(mmu_request_image_id),
        .request_valid(mmu_request_valid),
        .pixel_out(mmu_pixel_output)
    );

    logic buffer_ready;
    logic read_buffer;

    line_buffer_read_controller read_controller (
        .clk(vga_clk),
        .reset(system_reset),
        .x(x),
        .y(y),
        .video_on_raw(video_on_raw),

        .produced_sync(produced_sync),
        .consumed_toggle(consumed_toggle),

        .buffer_ready(buffer_ready),
        .read_buffer(read_buffer),
        .vga_started(vga_started)
    );

    logic read_enable;
    logic [PIXEL_BITS-1:0] pixel_out;

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

    line_buffer_bram #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE),
        .LINE_PIXELS(LINE_PIXELS),
        .PIXEL_BITS(PIXEL_BITS),
        .PIXEL_SLOT_BITS(PIXEL_SLOT_BITS)
    ) line_buffer (
        .write_buffer(write_buffer),
        .write_x(write_x),
        .write_enable(write_enable),
        .write_clk(system_clk),
        .pixel_in(pixel_in),
        .write_mask(write_mask),

        .read_buffer(read_buffer),
        .read_x(X_WIDTH'(x / HORIZONTAL_SCALE)),
        .read_enable(read_enable),
        .read_clk(vga_clk),
        .pixel_out(pixel_out)
    );
endmodule