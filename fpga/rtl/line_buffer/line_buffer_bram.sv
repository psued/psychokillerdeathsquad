module line_buffer_bram #(
    parameter int PIXELS_PER_CYCLE = 2, // Supported: 1, 2, or 4.
    parameter int LINE_PIXELS = 320, 
    parameter int PIXEL_BITS = 12, 
    parameter int PIXEL_SLOT_BITS = 16,

    localparam int BUFFER_COUNT = 2,
    localparam int X_WIDTH = $clog2(LINE_PIXELS),
    localparam int WORD_WIDTH = PIXEL_SLOT_BITS * PIXELS_PER_CYCLE,
    localparam int WORDS_PER_LINE = LINE_PIXELS / PIXELS_PER_CYCLE,
    localparam int TOTAL_WORDS = BUFFER_COUNT * WORDS_PER_LINE,
    localparam int ADDR_WIDTH = $clog2(TOTAL_WORDS),
    localparam int LANE_WIDTH = (PIXELS_PER_CYCLE > 1) ? $clog2(PIXELS_PER_CYCLE) : 1
)(
    input  logic         write_buffer,
    input  logic         [X_WIDTH-1:0] write_x,
    input  logic         write_enable,
    input  logic         write_clk,
    input  logic         [WORD_WIDTH-1:0] pixel_in,
    input  logic         [PIXELS_PER_CYCLE-1:0] write_mask,

    input  logic         read_buffer,
    input  logic         [X_WIDTH-1:0] read_x,
    input  logic         read_enable,
    input  logic         read_clk,
    output logic         [PIXEL_BITS-1:0] pixel_out
    );

    logic [WORD_WIDTH-1:0] buffer [0:TOTAL_WORDS-1]; 

    logic [ADDR_WIDTH-1:0] write_addr;
    logic [ADDR_WIDTH-1:0] read_addr;
    logic [WORD_WIDTH-1:0] read_word;
    logic [LANE_WIDTH-1:0] read_lane;

    assign write_addr = (write_buffer * WORDS_PER_LINE) + write_x / PIXELS_PER_CYCLE;
    assign read_addr  = (read_buffer * WORDS_PER_LINE) + read_x / PIXELS_PER_CYCLE;

    always_ff @(posedge write_clk) begin
        if (write_enable) begin
            for(int lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
                if (write_mask[lane]) begin
                    buffer[write_addr][lane*PIXEL_SLOT_BITS +: PIXEL_SLOT_BITS] <= pixel_in[lane*PIXEL_SLOT_BITS +: PIXEL_SLOT_BITS];
                end
            end
        end
    end

    always_ff @(posedge read_clk) begin
        if (read_enable) begin
            read_word <= buffer[read_addr];
            read_lane <= read_x % PIXELS_PER_CYCLE;
        end
    end

    assign pixel_out = read_word[read_lane*PIXEL_SLOT_BITS +: PIXEL_BITS];
endmodule
