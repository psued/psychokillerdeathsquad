module pixel_pipeline
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,
    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE,
    localparam int PENDING_WIDTH = $clog2(LAYER_COUNT * LINE_PIXELS + 1)
)(
    input logic clk,
    input logic reset,

    input logic stall,
    input logic clear_valid,
    input logic [X_COORDINATE_WIDTH-1:0] clear_x,
    input logic mmu_request_valid,
    input logic mmu_request_ready,
    input logic mmu_response_valid,
    input logic [X_COORDINATE_WIDTH-1:0] mmu_response_x,
    input logic [PIXELS_PER_CYCLE-1:0] mmu_response_mask,
    input logic [PIXEL_BITS-1:0] mmu_pixel_output [0:PIXELS_PER_CYCLE-1],

    output logic clear_ready,
    output logic mmu_response_ready,
    output logic writeback_empty,
    output logic [X_COORDINATE_WIDTH-1:0] bram_write_x,
    output logic bram_write_enable,
    output logic [WORD_WIDTH-1:0] bram_pixel_in,
    output logic [PIXELS_PER_CYCLE-1:0] bram_write_mask
    );

    logic [PENDING_WIDTH-1:0] pending_groups;
    logic request_fire;
    logic response_fire;
    logic clear_fire;

    assign request_fire = mmu_request_valid && mmu_request_ready;
    assign clear_ready = !reset && !stall;
    assign clear_fire = clear_valid && clear_ready;
    assign mmu_response_ready = !reset && !stall && !clear_valid;
    assign response_fire = mmu_response_valid && mmu_response_ready;
    assign writeback_empty = (pending_groups == '0);

    always_ff @(posedge clk) begin
        if (reset) begin
            pending_groups <= '0;
        end
        else if (request_fire && !response_fire) begin
            pending_groups <= pending_groups + 1'b1;
        end
        else if (response_fire && !request_fire) begin
            pending_groups <= pending_groups - 1'b1;
        end
    end

    assign bram_write_x = clear_valid ? clear_x : mmu_response_x;
    assign bram_write_enable = clear_fire || response_fire;

    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign bram_write_mask[lane] = clear_valid
            ? clear_fire && ((clear_x + lane) < LINE_PIXELS)
            : response_fire && mmu_response_mask[lane] && (mmu_pixel_output[lane] != TRANSPARENT_COLOR);
        assign bram_pixel_in[lane * PIXEL_SLOT_BITS +: PIXEL_SLOT_BITS] =
            clear_valid ? '0 : mmu_pixel_output[lane];
    end

endmodule