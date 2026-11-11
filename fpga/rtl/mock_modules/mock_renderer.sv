module mock_renderer #(
    parameter int PIXELS_PER_CYCLE = 2,
    parameter int LINE_PIXELS = 320,
    parameter int FRAME_LINES = 240,
    parameter int CHANNEL_BITS = 4,
    parameter int PIXEL_SLOT_BITS = 16,

    localparam int X_WIDTH = $clog2(LINE_PIXELS),
    localparam int Y_WIDTH = $clog2(FRAME_LINES),
    localparam int PIXEL_BITS = 3 * CHANNEL_BITS,
    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE,
    localparam int LAST_GROUP_X = LINE_PIXELS - PIXELS_PER_CYCLE,
    localparam int LAST_LINE = FRAME_LINES - 1,
    localparam int COLOR_LEVELS = 2 ** CHANNEL_BITS,
    localparam logic [CHANNEL_BITS-1:0] CHANNEL_MAX = '1
)(
    input logic clk,
    input logic reset,
    input logic stall,

    output logic [X_WIDTH-1:0] write_x,
    output logic [X_WIDTH-1:0] render_x,
    output logic       write_enable,
    output logic [WORD_WIDTH-1:0] pixel_in,
    output logic [PIXELS_PER_CYCLE-1:0] write_mask,
    output logic       line_done
);

    logic [Y_WIDTH-1:0] render_line;
    logic [CHANNEL_BITS-1:0] gradient_y;

    always_ff @(posedge clk) begin
        if (reset) begin
                render_x <= '0;
                render_line <= '0;
                line_done <= 1'b0;
            end
        else begin
            line_done <= 1'b0;

            if (!stall && !line_done) begin
                if(render_x == LAST_GROUP_X) begin
                    line_done <= 1'b1;
                    render_x <= '0;

                    if(render_line == LAST_LINE)
                        render_line <= '0;
                    else
                        render_line <= render_line + 1'b1;
                end
                else begin
                    render_x <= render_x + PIXELS_PER_CYCLE;
                end
            end
        end
    end

    assign write_x = render_x;
    assign write_enable = !reset && !stall && !line_done;
    assign write_mask = '1;

    // Sixteen color levels across each visible axis.
    assign gradient_y = (render_line * COLOR_LEVELS) / FRAME_LINES;

    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin : pixel_lanes
        logic [CHANNEL_BITS-1:0] gradient_x;
        logic [PIXEL_BITS-1:0] pixel_rgb;


        assign gradient_x = ((render_x + lane) * COLOR_LEVELS) / LINE_PIXELS;
        
        assign pixel_rgb = {
            gradient_y,          // red increases downward
            gradient_x,          // green increases to the right
            CHANNEL_MAX - gradient_y    // blue decreases downward
        };
        
        assign pixel_in[lane*PIXEL_SLOT_BITS +: PIXEL_SLOT_BITS] = pixel_rgb;
    end
endmodule
