module pixel_pipeline
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,
    parameter int LATENCY = 3,

    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE
)(
    input logic clk,
    input logic reset,

    input logic [X_COORDINATE_WIDTH-1:0] destination_x,
    input logic [PIXELS_PER_CYCLE-1:0] pixel_group_mask,
    input logic line_issue_finished,
    input logic mmu_request_valid,
    input logic [PIXEL_BITS-1:0] mmu_pixel_output [0:PIXELS_PER_CYCLE-1],

    output logic [X_COORDINATE_WIDTH-1:0] bram_write_x,
    output logic bram_write_enable,
    output logic [WORD_WIDTH-1:0] bram_pixel_in,
    output logic [PIXELS_PER_CYCLE-1:0] bram_write_mask,
    output logic line_done
    );

    logic [X_COORDINATE_WIDTH-1:0] x_pipeline [0:LATENCY-1];
    logic [PIXELS_PER_CYCLE-1:0] mask_pipeline [0:LATENCY-1];
    logic line_done_pipeline [0:LATENCY-1];
    logic write_enable_pipeline [0:LATENCY-1];

    always_ff @(posedge clk) begin
        if(reset) begin
            for (int stage = 0; stage < LATENCY; stage ++) begin
                x_pipeline[stage] <= '0;
                mask_pipeline[stage] <= '0;
                line_done_pipeline[stage] <= '0;
                write_enable_pipeline[stage] <= '0;
            end
        end
        else begin
            x_pipeline[0] <= destination_x;
            mask_pipeline[0] <= pixel_group_mask;
            line_done_pipeline[0] <= line_issue_finished;
            write_enable_pipeline[0] <= mmu_request_valid;

            for (int stage = 1; stage < LATENCY; stage++) begin
                x_pipeline[stage] <= x_pipeline[stage-1];
                mask_pipeline[stage] <= mask_pipeline[stage-1];
                line_done_pipeline[stage] <= line_done_pipeline[stage-1];
                write_enable_pipeline[stage] <= write_enable_pipeline[stage-1];
            end
        end
    end

    assign bram_write_x = x_pipeline[LATENCY-1];
    assign bram_write_enable = write_enable_pipeline[LATENCY-1];

    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign bram_write_mask[lane] = mask_pipeline[LATENCY-1][lane] && (mmu_pixel_output[lane] != TRANSPARENT_COLOR);
        assign bram_pixel_in[lane * PIXEL_SLOT_BITS +: PIXEL_SLOT_BITS] = mmu_pixel_output[lane];
    end

    assign line_done = line_done_pipeline[LATENCY-1];

endmodule