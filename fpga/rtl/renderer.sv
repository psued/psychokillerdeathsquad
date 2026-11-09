module renderer
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,

    localparam int LATENCY = 3,
    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE,
    localparam int LAST_GROUP_X = LINE_PIXELS - PIXELS_PER_CYCLE,
    localparam int LAST_LINE = FRAME_LINES - 1
)(
    input logic clk,
    input logic reset,
    input logic stall,

    input layer_descriptor_t snapshot [0:LAYER_COUNT-1],

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
    output logic rendering
    );


    //CONTROL LOGIC

    logic [X_COORDINATE_WIDTH-1:0] x_counter;    //x-counter per layer, counts from 0 up to the width of the asset for that layer
    logic [Y_COORDINATE_WIDTH-1:0] y_counter;
    logic [LAYER_INDEX_WIDTH-1:0] layer_counter;
    logic [15:0] line_end;

    assign line_end = ((snapshot[layer_counter].width - 1) / PIXELS_PER_CYCLE) * PIXELS_PER_CYCLE;


    //TODO: Implement layer intersection logic
    logic layer_row_intersect;
    logic layer_disabled;
    logic layer_finished;
    logic layer_skip;
    logic last_layer;
    logic line_issue_finished;
    logic draining;
    logic issuing_row;

    assign layer_row_intersect = ((y_counter >= snapshot[layer_counter].dst_y) && (y_counter < snapshot[layer_counter].dst_y + snapshot[layer_counter].height));
    assign layer_disabled = ((snapshot[layer_counter].flags & FLAG_ENABLED) == 0);
    assign layer_finished = (x_counter == line_end);
    assign layer_skip = !layer_row_intersect || ((x_counter == 0) && layer_disabled);
    assign last_layer = layer_counter == (LAYER_COUNT-1);
    assign line_issue_finished = !reset && issuing_row && !stall && last_layer && (layer_skip || layer_finished);


    always_ff @(posedge clk) begin
        if(reset) begin
            draining <= 1'b0;
        end 
        else if (line_issue_finished) begin
            draining <= 1'b1;
        end
        else if (line_done) begin
            draining <= 1'b0;
        end
    end


    always_ff @(posedge clk) begin
        if(reset) begin
            issuing_row <= 1'b0;
        end 
        else if (line_issue_finished) begin
            issuing_row <= 1'b0;
        end
        else if (!stall && !draining) begin
            issuing_row <= 1'b1;
        end
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            x_counter <= '0;
            y_counter <= '0;
            layer_counter <= '0;
        end
        else if (!stall && issuing_row) begin 
            if (layer_skip || layer_finished) begin
                if (last_layer) begin
                    layer_counter <= '0;

                    if (y_counter == FRAME_LINES - 1) begin
                        y_counter <= '0;
                    end
                    else begin
                        y_counter <= y_counter + 1'b1;
                    end
                end
                else begin
                    layer_counter <= layer_counter + 1'b1;
                end
            
                x_counter <= '0;
            end
            else begin
                x_counter <= x_counter + PIXELS_PER_CYCLE;
            end
        end
    end

    assign rendering = issuing_row || draining;

    //MMU ASSIGNMENTS

    assign mmu_request_image_id = snapshot[layer_counter].image_id;
    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign mmu_request_x[lane] = x_counter + lane + snapshot[layer_counter].src_x;
    end
    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign mmu_request_y[lane] = y_counter - snapshot[layer_counter].dst_y + snapshot[layer_counter].src_y;
    end

    
    assign mmu_request_valid = !reset && !stall && issuing_row && !layer_skip; 


    //PIPELINE REGISTERS
    logic [X_COORDINATE_WIDTH-1:0] destination_x;
    logic [PIXELS_PER_CYCLE-1:0] pixel_group_mask;

    assign destination_x = x_counter + snapshot[layer_counter].dst_x;

    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign pixel_group_mask[lane] = !reset && issuing_row && ((x_counter + lane) < snapshot[layer_counter].width);
    end

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


    //BRAM WRITES

    assign bram_write_x = x_pipeline[LATENCY-1];
    assign bram_write_enable = write_enable_pipeline[LATENCY-1];

    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign bram_write_mask[lane] = mask_pipeline[LATENCY-1][lane] && (mmu_pixel_output[lane] != TRANSPARENT_COLOR);
    end

    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign bram_pixel_in[lane * PIXEL_SLOT_BITS +: PIXEL_SLOT_BITS] = mmu_pixel_output[lane];
    end

    assign line_done = line_done_pipeline[LATENCY-1];
endmodule