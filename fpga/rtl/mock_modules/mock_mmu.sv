module mock_mmu #(
    parameter int LATENCY = 3,
    parameter int PIXELS_PER_CYCLE = 2,

    localparam int X_WIDTH = 16,
    localparam int Y_WIDTH = 16,
    localparam int IMAGE_ID_WIDTH = 16,
    localparam int PIXEL_BITS = 12
)(
    input logic clk,
    input logic reset,
    input logic [X_WIDTH-1:0] x [0:PIXELS_PER_CYCLE-1],
    input logic [Y_WIDTH-1:0] y [0:PIXELS_PER_CYCLE-1],
    input logic [IMAGE_ID_WIDTH-1:0] image_id,
    input logic request_valid,

    output logic [PIXEL_BITS-1:0] pixel_out [0:PIXELS_PER_CYCLE-1]
    );

    localparam logic [PIXEL_BITS-1:0] TRANSPARENT_COLOR = 12'hF0F;
    localparam logic [PIXEL_BITS-1:0] BACKGROUND_COLOR = 12'h124;
    localparam int CHECKER_SIZE = 64;
    localparam int GRADIENT_WIDTH = 96;
    localparam int GRADIENT_HEIGHT = 64;

    logic [PIXEL_BITS-1:0] pixel_pipeline
        [0:LATENCY-1][0:PIXELS_PER_CYCLE-1];

    function automatic logic [PIXEL_BITS-1:0] make_pixel(
        input logic [IMAGE_ID_WIDTH-1:0] asset_id,
        input logic [X_WIDTH-1:0] source_x,
        input logic [Y_WIDTH-1:0] source_y
    );
        make_pixel = TRANSPARENT_COLOR;

        case (asset_id)
            16'd0: make_pixel = BACKGROUND_COLOR;

            16'd1: begin
                if (source_x < CHECKER_SIZE && source_y < CHECKER_SIZE) begin
                    if (source_x[3] ^ source_y[3])
                        make_pixel = 12'hFFF;
                end
            end

            16'd2: begin
                if (source_x < GRADIENT_WIDTH && source_y < GRADIENT_HEIGHT) begin
                    make_pixel = {
                        4'((int'(source_x) * 16) / GRADIENT_WIDTH),
                        4'((int'(source_y) * 16) / GRADIENT_HEIGHT),
                        4'h4
                    };
                end
            end

            default: make_pixel = TRANSPARENT_COLOR;
        endcase
    endfunction

    always_ff @(posedge clk) begin
        if (reset) begin
            for (int stage = 0; stage < LATENCY; stage++) begin
                for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++)
                    pixel_pipeline[stage][lane] <= TRANSPARENT_COLOR;
            end
        end else begin
            for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
                pixel_pipeline[0][lane] <= request_valid
                    ? make_pixel(image_id, x[lane], y[lane])
                    : TRANSPARENT_COLOR;
            end

            for (int stage = 1; stage < LATENCY; stage++) begin
                for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++)
                    pixel_pipeline[stage][lane] <= pixel_pipeline[stage-1][lane];
            end
        end
    end

    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin : output_lanes
        assign pixel_out[lane] = pixel_pipeline[LATENCY-1][lane];
    end

endmodule