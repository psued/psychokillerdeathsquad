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
    logic vga_reset;
    logic reset_request;
    logic vga_timing_reset;
    logic vga_started;

    (* ASYNC_REG = "TRUE" *) logic [1:0] system_reset_pipe = 2'b11;
    (* ASYNC_REG = "TRUE" *) logic [1:0] vga_reset_pipe = 2'b11;

    assign reset_request = reset | ~clk_locked;

    // Assert immediately; release on each domain's clock.
    always_ff @(posedge system_clk or posedge reset_request) begin
        if (reset_request)
            system_reset_pipe <= 2'b11;
        else
            system_reset_pipe <= {system_reset_pipe[0], 1'b0};
    end

    always_ff @(posedge vga_clk or posedge reset_request) begin
        if (reset_request)
            vga_reset_pipe <= 2'b11;
        else
            vga_reset_pipe <= {vga_reset_pipe[0], 1'b0};
    end

    assign system_reset = system_reset_pipe[1];
    assign vga_reset = vga_reset_pipe[1];
    assign vga_timing_reset = vga_reset | ~vga_started;

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

    renderer_pkg::layer_descriptor_t mmu_current_layer;
    logic snapshot_start;
    logic snapshot_ready;
    logic frame_setup;
    logic fetcher_idle;
    logic [15:0] cached_asset_width;
    logic [7:0] cached_bits_per_color;
    logic [renderer_pkg::LAYER_INDEX_WIDTH-1:0] gamestate_layer;
    logic [renderer_pkg::LAYER_INDEX_WIDTH-1:0] mmu_layer;
    logic [15:0] mmu_read_address;
    logic [23:0] mmu_data;
    logic [15:0] mmu_asset_width;
    logic [7:0] mmu_bits_per_color;
    logic mmu_enabled;
    logic mmu_flipped;

    wire [15:0] debug_mmu_width;
    wire [15:0] debug_cached_width;
    wire [7:0] debug_cached_flags;

    wire debug_mcu_ready;
    wire debug_mmu_state_written;
    wire debug_mmu_selected_mem;
    wire [15:0] debug_mmu_file_name;
    wire debug_mmu_reading_file;
    wire [31:0] debug_mmu_sd_byte_counter;
    wire [31:0] debug_mmu_file_size;

    wire debug_sd_reading;
    wire debug_fs_read;
    wire debug_file_addr_fetched;
    wire debug_sd_cmd_ready;
    wire debug_sd_awaiting_resp;
    wire debug_sd_byte_ready;
    wire debug_sd_error;
    wire [2:0] debug_sd_cmd12_state;
    wire [5:0] debug_sd_cmd12_bits_left;

    assign debug_mmu_width = mmu_current_layer.width;
    assign debug_cached_width = current_layer.width;
    assign debug_cached_flags = current_layer.flags;

    assign debug_mcu_ready = MCU_READY;
    assign debug_mmu_state_written = real_memory.state.state_written;
    assign debug_mmu_selected_mem = real_memory.state.selected_mem;
    assign debug_mmu_file_name = real_memory.state.file_name;
    assign debug_mmu_reading_file = real_memory.state.reading_file;
    assign debug_mmu_sd_byte_counter = real_memory.state.sd_byte_counter;
    assign debug_mmu_file_size = real_memory.state.file_size;

    assign debug_sd_reading = real_memory.state.sd_reading;
    assign debug_fs_read = real_memory.state.fs_read;
    assign debug_file_addr_fetched = real_memory.state.file_addr_fetched;
    assign debug_sd_cmd_ready = real_memory.state.sd.cmd_ready;
    assign debug_sd_awaiting_resp = real_memory.state.sd.awaiting_resp;
    assign debug_sd_byte_ready = real_memory.state.sd_byte_ready;
    assign debug_sd_error = real_memory.state.sd.error;
    assign debug_sd_cmd12_state = real_memory.state.sd.cmd12_state;
    assign debug_sd_cmd12_bits_left = real_memory.state.sd.cmd12_bits_left;

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
        .snapshot_ready(snapshot_ready),
        .snapshot_start(snapshot_start),
        .frame_setup(frame_setup),

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

    snapshot_cache snapshot_cache_inst (
        .clk(system_clk),
        .reset(system_reset),
        .load_start(snapshot_start),
        .fetcher_idle(fetcher_idle),
        .source_layer(mmu_current_layer),
        .source_asset_width(mmu_asset_width),
        .source_bits_per_color(mmu_bits_per_color),
        .layer_index(layer_counter),
        .fetch_layer_index(mmu_layer),
        .mmu_layer(gamestate_layer),
        .current_layer(current_layer),
        .asset_width(cached_asset_width),
        .bits_per_color(cached_bits_per_color),
        .ready(snapshot_ready)
    );

    asset_fetcher #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) asset_fetcher_inst (
        .clk(system_clk),
        .reset(system_reset),
        .pause(frame_setup),
        .idle(fetcher_idle),
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
        .width(current_layer.width),
        .asset_width(cached_asset_width),
        .height(current_layer.height),
        .src_x(current_layer.src_x),
        .src_y(current_layer.src_y),
        .dst_x(current_layer.dst_x),
        .dst_y(current_layer.dst_y),
        .z(current_layer.z),
        .enabled((current_layer.flags & renderer_pkg::FLAG_ENABLED) != 0),
        .flipped((current_layer.flags & renderer_pkg::FLAG_FLIP_X) != 0),
        .bits_per_color(cached_bits_per_color),
        .mmu_layer(mmu_layer),
        .read_address(mmu_read_address)
    );

    logic buffer_ready;
    logic read_buffer;

    line_buffer_read_controller read_controller (
        .clk(vga_clk),
        .reset(vga_reset),
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
        .reset(vga_reset),

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
        begin : real_memory
            assign mmu_current_layer.image_id = '0;
            assign mmu_current_layer.flags =
                8'h00 | (mmu_flipped << 1) | mmu_enabled;

            // power-on reset
            logic por = 1;
            logic rst = reset || por;
            always_ff @ (posedge system_clk)
                por <= 0;

            gamestate state(
                .clk(system_clk),
                .rst(system_reset),

                .layer(gamestate_layer),
                .width(mmu_current_layer.width),
                .height(mmu_current_layer.height),
                .src_x(mmu_current_layer.src_x),
                .src_y(mmu_current_layer.src_y),
                .dst_x(mmu_current_layer.dst_x),
                .dst_y(mmu_current_layer.dst_y),
                .z(mmu_current_layer.z),
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
