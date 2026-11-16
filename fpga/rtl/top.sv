module top #(
    parameter int PIXELS_PER_CYCLE = 2,
    parameter bit USE_MOCK_MMU = 1'b0,

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
    output logic [CHANNEL_BITS-1:0] blue,


    output logic       SD_SCLK,
    output logic       SD_DI,
    input  logic       SD_DO,
    output logic       SD_CS,

    input  logic       MCU_SCLK,
    output logic       MCU_MISO,
    input  logic       MCU_MOSI,
    input  logic       MCU_CS,
    output logic       MCU_READY

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
    logic [9:0] x; // VGA raster: 0?799, including blanking.
    logic [9:0] y; // VGA raster: 0?524, including blanking.

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
    logic frame_done;
    logic write_enable;
    logic [WORD_WIDTH-1:0] pixel_in;
    logic [PIXELS_PER_CYCLE-1:0] write_mask;
    logic stall;

    renderer_pkg::layer_descriptor_t current_layer;
    logic [renderer_pkg::LAYER_INDEX_WIDTH-1:0] layer_counter;
    logic [renderer_pkg::X_COORDINATE_WIDTH-1:0] mmu_request_x;
    logic [renderer_pkg::Y_COORDINATE_WIDTH-1:0] mmu_y_select;
    logic [PIXELS_PER_CYCLE-1:0] mmu_request_mask;
    logic mmu_prepare_valid;
    logic mmu_prepare_ready;
    logic mmu_request_valid;
    logic mmu_request_ready;
    logic mmu_response_valid;
    logic mmu_response_ready;
    logic [renderer_pkg::X_COORDINATE_WIDTH-1:0] mmu_response_x;
    logic [PIXELS_PER_CYCLE-1:0] mmu_response_mask;
    logic [PIXEL_BITS-1:0] mmu_pixel_output [0:PIXELS_PER_CYCLE-1];

    renderer_pkg::layer_descriptor_t fetch_layer;
    logic [renderer_pkg::LAYER_INDEX_WIDTH-1:0] mmu_layer;
    logic [15:0] mmu_read_address;
    logic [23:0] mmu_data;
    logic [15:0] mmu_asset_width;
    logic [7:0] mmu_bits_per_color;
    logic mmu_enabled;
    logic mmu_flipped;

    line_buffer_write_controller write_controller (
        .clk(system_clk),
        .reset(system_reset),

        .line_done(line_done),

        .consumed_sync(consumed_sync),
        .produced_toggle(produced_toggle),

        .write_buffer(write_buffer),
        .stall(stall)
    );

    renderer #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) renderer_inst (
        .clk(system_clk),
        .reset(system_reset),
        .stall(stall),
        .current_layer(current_layer),
        .layer_counter(layer_counter),

        .mmu_request_x(mmu_request_x),
        .mmu_y_select(mmu_y_select),
        .mmu_request_mask(mmu_request_mask),
        .mmu_prepare_valid(mmu_prepare_valid),
        .mmu_prepare_ready(mmu_prepare_ready),
        .mmu_request_valid(mmu_request_valid),
        .mmu_request_ready(mmu_request_ready),
        .mmu_response_valid(mmu_response_valid),
        .mmu_response_ready(mmu_response_ready),
        .mmu_response_x(mmu_response_x),
        .mmu_response_mask(mmu_response_mask),
        .mmu_pixel_output(mmu_pixel_output),

        .bram_write_x(write_x),
        .bram_write_enable(write_enable),
        .bram_pixel_in(pixel_in),
        .bram_write_mask(write_mask),
        .line_done(line_done),
        .frame_done(frame_done)
    );

    asset_fetcher #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) asset_fetcher_inst (
        .clk(system_clk),
        .reset(system_reset),
        .data(mmu_data),
        .y_select(mmu_y_select),
        .prepare_valid(mmu_prepare_valid),
        .prepare_ready(mmu_prepare_ready),
        .request_valid(mmu_request_valid),
        .request_ready(mmu_request_ready),
        .request_x(mmu_request_x),
        .request_mask(mmu_request_mask),
        .response_valid(mmu_response_valid),
        .response_ready(mmu_response_ready),
        .response_x(mmu_response_x),
        .response_mask(mmu_response_mask),
        .pixel_output(mmu_pixel_output),

        .layer(layer_counter),
        .width(fetch_layer.width),
        .asset_width(mmu_asset_width),
        .height(fetch_layer.height),
        .src_x(fetch_layer.src_x),
        .src_y(fetch_layer.src_y),
        .dst_x(fetch_layer.dst_x),
        .dst_y(fetch_layer.dst_y),
        .z(fetch_layer.z),
        .enabled((fetch_layer.flags & renderer_pkg::FLAG_ENABLED) != 0),
        .flipped((fetch_layer.flags & renderer_pkg::FLAG_FLIP_X) != 0),
        .bits_per_color(mmu_bits_per_color),
        .mmu_layer(mmu_layer),
        .read_address(mmu_read_address)
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


    generate
        if (USE_MOCK_MMU) begin : mock_memory
            mock_snapshot snapshot_source (
                .layer_counter(layer_counter),
                .current_layer(current_layer)
            );

            mock_mmu memory_source (
                .clk(system_clk),
                .reset(system_reset),
                .layer(mmu_layer),
                .read_address(mmu_read_address),
                .current_layer(fetch_layer),
                .asset_width(mmu_asset_width),
                .bits_per_color(mmu_bits_per_color),
                .data(mmu_data)
            );

            assign SD_SCLK = 1'b0;
            assign SD_DI = 1'b1;
            assign SD_CS = 1'b1;
            assign MCU_MISO = 1'b0;
            assign MCU_READY = 1'b0;
        end
        else begin : real_memory
            assign fetch_layer = current_layer;
            assign current_layer.flags =
                8'h00 | (mmu_flipped << 1) | mmu_enabled;

            // power-on reset
            logic por = 1;
            logic rst = reset || por;
            always_ff @ (posedge system_clk)
                por <= 0;

            gamestate state(
                .clk(system_clk),
                .rst(rst),

                .layer(layer_counter),
                .width(current_layer.width),
                .height(current_layer.height),
                .src_x(current_layer.src_x),
                .src_y(current_layer.src_y),
                .dst_x(current_layer.dst_x),
                .dst_y(current_layer.dst_y),
                .z(current_layer.z),
                .enabled(mmu_enabled),
                .flipped(mmu_flipped),
                .bits_per_color(mmu_bits_per_color),
                .asset_width(mmu_asset_width),

                .addr(mmu_read_address),
                .data(mmu_data),

        .frame_rendered(frame_done),

                .SD_SCLK(SD_SCLK),
                .SD_DI(SD_DI),
                .SD_DO(SD_DO),
                .SD_CS(SD_CS),

                .MCU_SCLK(MCU_SCLK),
                .MCU_MISO(MCU_MISO),
                .MCU_MOSI(MCU_MOSI),
                .MCU_CS(MCU_CS),
                .MCU_READY(MCU_READY)
            );
        end
    endgenerate
endmodule
