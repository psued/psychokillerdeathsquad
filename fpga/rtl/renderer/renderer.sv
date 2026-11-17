module renderer
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,

    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE
)(
    input logic clk,
    input logic reset,
    input logic stall,

    input layer_descriptor_t current_layer,
    input logic snapshot_ready,
    output logic snapshot_start,
    output logic frame_setup,

    //MMU I/O
    output logic [X_COORDINATE_WIDTH-1:0] mmu_request_x,
    output logic [Y_COORDINATE_WIDTH-1:0] mmu_y_select,
    output logic [PIXELS_PER_CYCLE-1:0] mmu_request_mask,
    output logic mmu_prepare_valid,
    output logic mmu_request_valid,
    output logic mmu_response_ready,
    input logic mmu_prepare_ready,
    input logic mmu_request_ready,
    input logic mmu_response_valid,
    input logic [X_COORDINATE_WIDTH-1:0] mmu_response_x,
    input logic [PIXELS_PER_CYCLE-1:0] mmu_response_mask,
    input logic [PIXEL_BITS-1:0] mmu_pixel_output [0:PIXELS_PER_CYCLE-1],

    //BRAM I/O              
    output logic [X_COORDINATE_WIDTH-1:0] bram_write_x,
    output logic bram_write_enable,
    output logic [WORD_WIDTH-1:0] bram_pixel_in,
    output logic [PIXELS_PER_CYCLE-1:0] bram_write_mask,

    output logic line_done,  
    output logic frame_done,
    output logic [LAYER_INDEX_WIDTH-1:0] layer_counter
    );

    logic [X_COORDINATE_WIDTH-1:0] x_counter;
    logic [Y_COORDINATE_WIDTH-1:0] y_counter;

    layer_descriptor_t current_layer_reg;
    logic layer_load;
    logic writeback_empty;
    logic clear_valid;
    logic clear_ready;
    logic sort_start;
    logic sort_done;
    logic [LAYER_INDEX_WIDTH-1:0] layer_order_counter;
    logic [LAYER_INDEX_WIDTH-1:0] sort_read_layer;
    logic [LAYER_INDEX_WIDTH-1:0] sorted_layer;

    assign mmu_y_select = y_counter;
    assign layer_counter = frame_setup ? sort_read_layer : sorted_layer;

    always_ff @(posedge clk) begin
        if (reset) begin
            current_layer_reg <= '0;
        end
        else if (layer_load) begin
            current_layer_reg <= current_layer;
        end
    end

    layer_sorter sorter (
        .clk(clk),
        .reset(reset),
        .start(sort_start),
        .layer_z(current_layer.z),
        .order_index(layer_order_counter),
        .read_layer_index(sort_read_layer),
        .sorted_layer_index(sorted_layer),
        .done(sort_done)
    );

    scanline_scheduler #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) scheduler (
        .clk(clk),
        .reset(reset),
        .stall(stall),
        .writeback_empty(writeback_empty),
        .clear_ready(clear_ready),
        .sort_done(sort_done),
        .snapshot_ready(snapshot_ready),
        .mmu_prepare_ready(mmu_prepare_ready),
        .mmu_request_ready(mmu_request_ready),
        .current_layer(current_layer_reg),

        .x_counter(x_counter),
        .y_counter(y_counter),
        .layer_order_counter(layer_order_counter),
        .layer_load(layer_load),
        .snapshot_start(snapshot_start),
        .sort_start(sort_start),
        .frame_setup(frame_setup),
        .clear_valid(clear_valid),
        .mmu_prepare_valid(mmu_prepare_valid),
        .mmu_request_valid(mmu_request_valid),
        .line_done(line_done),
        .frame_done(frame_done)
    );
            
    layer_mapper #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) mapper (
        .x_counter(x_counter),
        .current_layer(current_layer_reg),
        .mmu_request_valid(mmu_request_valid),

        .pixel_group_mask(mmu_request_mask),
        .destination_x(mmu_request_x)
    );

    pixel_pipeline #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) pipeline (
        .clk(clk),
        .reset(reset),
        .stall(stall),
        .clear_valid(clear_valid),
        .clear_x(x_counter),
        .clear_ready(clear_ready),
        .mmu_request_valid(mmu_request_valid),
        .mmu_request_ready(mmu_request_ready),
        .mmu_response_valid(mmu_response_valid),
        .mmu_response_ready(mmu_response_ready),
        .mmu_response_x(mmu_response_x),
        .mmu_response_mask(mmu_response_mask),
        .mmu_pixel_output(mmu_pixel_output),
        .writeback_empty(writeback_empty),

        .bram_write_x(bram_write_x),
        .bram_write_enable(bram_write_enable),
        .bram_pixel_in(bram_pixel_in),
        .bram_write_mask(bram_write_mask)
    );
endmodule
