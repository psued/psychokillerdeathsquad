module renderer
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,
    parameter int LATENCY = 3,

    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE,
    localparam int LAST_GROUP_X = LINE_PIXELS - PIXELS_PER_CYCLE,
    localparam int LAST_LINE = FRAME_LINES - 1
)(
    input logic clk,
    input logic reset,
    input logic stall,

    input layer_descriptor_t current_layer,

    //MMU I/O
    output logic [IMAGE_ID_WIDTH-1:0] mmu_request_image_id,
    output logic [PIXEL_SLOT_BITS-1:0] mmu_request_x [0:PIXELS_PER_CYCLE-1],
    output logic [PIXEL_SLOT_BITS-1:0] mmu_request_y [0:PIXELS_PER_CYCLE-1],
    output logic mmu_request_valid,
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

    logic line_issue_finished;

    scanline_scheduler #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) scheduler (
        .clk(clk),
        .reset(reset),
        .stall(stall),
        .line_done(line_done),
        .current_layer(current_layer),

        .x_counter(x_counter),
        .y_counter(y_counter),
        .layer_counter(layer_counter),
        .mmu_request_valid(mmu_request_valid),
        .line_issue_finished(line_issue_finished),
        .frame_done(frame_done)
    );
            
    logic [PIXELS_PER_CYCLE-1:0] pixel_group_mask;
    logic [X_COORDINATE_WIDTH-1:0] destination_x;

    layer_mapper #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE)
    ) mapper (
        .x_counter(x_counter),
        .y_counter(y_counter),
        .current_layer(current_layer),
        .mmu_request_valid(mmu_request_valid),

        .mmu_request_image_id(mmu_request_image_id),
        .mmu_request_x(mmu_request_x),
        .mmu_request_y(mmu_request_y),
        .pixel_group_mask(pixel_group_mask),
        .destination_x(destination_x)
    );

    pixel_pipeline #(
        .PIXELS_PER_CYCLE(PIXELS_PER_CYCLE),
        .LATENCY(LATENCY)
    ) pipeline (
        .clk(clk),
        .reset(reset),
        .destination_x(destination_x),
        .pixel_group_mask(pixel_group_mask),
        .line_issue_finished(line_issue_finished),
        .mmu_request_valid(mmu_request_valid),
        .mmu_pixel_output(mmu_pixel_output),

        .bram_write_x(bram_write_x),
        .bram_write_enable(bram_write_enable),
        .bram_pixel_in(bram_pixel_in),
        .bram_write_mask(bram_write_mask),
        .line_done(line_done)
    );
endmodule