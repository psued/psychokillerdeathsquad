module asset_fetcher 
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,

    localparam int WORD_WIDTH = 24,
    localparam int COLOUR_WIDTH = 12,
    localparam int TOTAL_WORDS = 4,
    localparam int MAX_PALETTE_COLOURS = 32,
    localparam int MAX_INDEX_WIDTH = $clog2(MAX_PALETTE_COLOURS),
    localparam int MAX_ASSET_WIDTH = 600,
    localparam int MAX_ASSET_HEIGHT = 240,
    localparam int MAX_PIXELS = MAX_ASSET_WIDTH * MAX_ASSET_HEIGHT,
    localparam int MAX_PIXEL_WORDS = (MAX_PIXELS * MAX_INDEX_WIDTH + WORD_WIDTH - 1) / WORD_WIDTH
)(
    input logic clk,
    input logic reset,

    input logic [WORD_WIDTH-1:0] data,

    input logic [Y_COORDINATE_WIDTH-1:0] y_select,
    input logic prepare_valid,
    input logic request_valid,
    input logic response_ready,

    input logic [X_COORDINATE_WIDTH-1:0] request_x,
    input logic [PIXELS_PER_CYCLE-1:0] request_mask,

    //Gamestate Inputs
    input logic [4:0] layer,
    input logic [15:0] width,
    input logic [15:0] asset_width,
    input logic [15:0] height,
    input logic [15:0] src_x,
    input logic [15:0] src_y,
    input logic [15:0] dst_x,
    input logic [15:0] dst_y,
    input logic [7:0] z,
    input logic enabled,
    input logic flipped,
    input logic [7:0] bits_per_color,


    output logic [4:0] mmu_layer,
    output logic [15:0] read_address,

    output logic prepare_ready,
    output logic request_ready,
    output logic response_valid,
    output logic [X_COORDINATE_WIDTH-1:0] response_x,
    output logic [PIXELS_PER_CYCLE-1:0] response_mask,
    output logic [PIXEL_BITS-1:0] pixel_output [0:PIXELS_PER_CYCLE-1]
    );

    logic [15:0] src_x_reg;
    logic [15:0] src_y_reg;
    logic [15:0] dst_y_reg;
    logic [15:0] y_select_reg;
    logic [15:0] width_reg;
    logic [15:0] asset_width_reg;
    logic [7:0] bits_per_color_reg;
    logic flipped_reg;

    logic [$clog2(MAX_PALETTE_COLOURS + 1) - 1:0] palette_colours;
    logic [$clog2(MAX_PALETTE_COLOURS*COLOUR_WIDTH/WORD_WIDTH):0] palette_words;

    assign palette_colours = 1 << bits_per_color_reg;
    assign palette_words = (palette_colours * COLOUR_WIDTH + WORD_WIDTH - 1) / WORD_WIDTH;


    typedef enum logic [2:0] {
        IDLE,
        LOAD_PALETTE,
        BUFFER_FILL,
        STREAM,
        DRAIN
    } state_t;

    state_t state;
    state_t next_state;

    always_ff @(posedge clk) begin
        if (reset) begin
            state <= IDLE;
        end
        else begin
            state <= next_state;
        end
    end

    logic [15:0] read_address_reg;
    logic [4:0] active_layer;

    assign mmu_layer = (state == IDLE) ? layer : active_layer;

    logic [11:0] palette [0:31];
    logic [$clog2(MAX_PALETTE_COLOURS)-2:0] palette_word_counter;    

    always_ff @(posedge clk) begin
        if (reset) begin
            read_address_reg <= '0;
        end
        else begin
            read_address_reg <= read_address;
        end
    end

    logic read_enable;
    logic write_enable;
    logic [7:0] write_address;
    logic palette_reads_finished;
    logic last_palette_request;
    logic last_palette_response;

    assign last_palette_request = palette_word_counter == (palette_words - 1);
    assign last_palette_response = (read_address_reg == (palette_words - 1));

    
    //BUFFER FILL LOGIC
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] pixel_word_counter;

    logic [$clog2(MAX_PIXELS * MAX_INDEX_WIDTH)-1:0] first_pixel_bit;
    logic [$clog2(MAX_PIXELS * MAX_INDEX_WIDTH)-1:0] end_pixel_bit;
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] first_word_address;
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] last_word_address;
    logic [15:0] pixel_word_address;
    logic [$clog2(WORD_WIDTH)-1:0] first_bit_offset;
    logic [$clog2(WORD_WIDTH)-1:0] initial_bit_offset;
    logic [$clog2(WORD_WIDTH)-1:0] bit_offset;
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] words_needed;

    assign first_pixel_bit = ((src_y_reg + y_select_reg - dst_y_reg) * asset_width_reg + src_x_reg) * bits_per_color_reg;
    assign end_pixel_bit = first_pixel_bit + width_reg * bits_per_color_reg;
    assign first_word_address = palette_words + first_pixel_bit / WORD_WIDTH;
    assign last_word_address = (bits_per_color_reg == 0)
                             ? first_word_address
                             : palette_words + (end_pixel_bit - 1) / WORD_WIDTH;
    assign pixel_word_address = flipped_reg
                              ? last_word_address - pixel_word_counter
                              : first_word_address + pixel_word_counter;
    assign first_bit_offset = first_pixel_bit % WORD_WIDTH;
    // Skip bits outside the rectangle from the selected entry edge.
    assign initial_bit_offset = (bits_per_color_reg == 0)
                              ? '0
                              : (flipped_reg
                                 ? WORD_WIDTH - 1 - ((end_pixel_bit - 1) % WORD_WIDTH)
                                 : first_bit_offset);
    assign words_needed = (first_bit_offset + width_reg * bits_per_color_reg + WORD_WIDTH - 1) / WORD_WIDTH;

    logic [WORD_WIDTH-1:0] buffer [0:TOTAL_WORDS-1];
    logic [TOTAL_WORDS-1:0] word_status;
    logic [$clog2(TOTAL_WORDS)-1:0] read_pointer;
    logic [$clog2(TOTAL_WORDS)-1:0] write_pointer;
    logic [$clog2(TOTAL_WORDS)-1:0] write_pointer_reg;

    logic [$clog2(TOTAL_WORDS)-1:0] next_read_pointer;
    logic [2*WORD_WIDTH-1:0] pixel_window;

    assign next_read_pointer = 
        (read_pointer == TOTAL_WORDS-1)
        ? '0
        : read_pointer + 1'b1;

    assign pixel_window = flipped_reg
                        ? {buffer[next_read_pointer], buffer[read_pointer]}
                        : {buffer[read_pointer], buffer[next_read_pointer]};

    logic [MAX_INDEX_WIDTH-1:0] pixel_index [0:PIXELS_PER_CYCLE-1];
    logic [PIXEL_BITS-1:0] decoded_pixel [0:PIXELS_PER_CYCLE-1];

    int unsigned bits_consumed;
    int unsigned shift_amount;
    int unsigned advanced_bits;

    always_comb begin
        bits_consumed = 0;
        shift_amount = 0;

        for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
            pixel_index[lane] = '0;
            decoded_pixel[lane] = '0;

            if (request_mask[lane]) begin
                if(bits_per_color_reg == 0) begin
                    decoded_pixel[lane] = palette[0];
                end
                else begin
                    shift_amount = flipped_reg
                                 ? bit_offset + bits_consumed
                                 : 2*WORD_WIDTH - bit_offset - bits_consumed - bits_per_color_reg;
                    pixel_index[lane] = (pixel_window >> shift_amount) & ((1 << bits_per_color_reg) - 1);
                    decoded_pixel[lane] = palette[pixel_index[lane]];
                    bits_consumed = bits_consumed + bits_per_color_reg;
                end
            end
        end
    end

    assign advanced_bits = bit_offset + bits_consumed;

    logic buffer_has_data;
    logic response_space;

    assign buffer_has_data = 
        (bits_consumed == 0) || 
        (word_status[read_pointer] &&
        ((bit_offset + bits_consumed <= WORD_WIDTH) ||
        word_status[next_read_pointer]));

    assign response_space = !response_valid || response_ready;


    always_ff @(posedge clk) begin
        if (reset) begin
            active_layer <= '0;
            flipped_reg <= 1'b0;
            palette_word_counter <= '0;
            write_enable <= '0;
            palette_reads_finished <= '0;

            word_status <= '0;
            read_pointer <= '0;
            write_pointer <= '0;
            response_valid <= '0;

        end
        else begin
            write_enable <= read_enable;

            if (response_valid && response_ready) begin
                response_valid <= 1'b0;
            end

            if(state == IDLE) begin
                if(prepare_valid && prepare_ready) begin
                    active_layer <= layer;
                    src_x_reg <= src_x;
                    src_y_reg <= src_y;
                    dst_y_reg <= dst_y;
                    y_select_reg <= y_select;
                    width_reg <= width;
                    asset_width_reg <= asset_width;
                    bits_per_color_reg <= bits_per_color;
                    flipped_reg <= flipped;

                    palette_word_counter <= '0;
                    palette_reads_finished <= 1'b0;
                end
            end
            else if (state == LOAD_PALETTE) begin
                if(read_enable) begin
                    if(last_palette_request) begin
                        palette_reads_finished <= 1'b1;
                    end
                    else begin
                        palette_word_counter <= palette_word_counter + 1'b1;
                    end
                end

                if(write_enable) begin
                    palette[read_address_reg*2] <= data[23:12];
                    palette[read_address_reg*2 + 1] <= data[11:0];
                end

                if(write_enable && last_palette_response) begin
                    write_pointer <= '0;
                    write_pointer_reg <= '0;
                    read_pointer <= '0;
                    bit_offset <= initial_bit_offset;
                    word_status <= '0;
                    pixel_word_counter <= '0;
                end
            end
            else if (state == BUFFER_FILL || state == STREAM) begin
                write_pointer_reg <= write_pointer;
                if (read_enable) begin
                    write_pointer <= write_pointer + 1'b1;
                    pixel_word_counter <= pixel_word_counter + 1;
                end

                if(write_enable) begin
                    word_status[write_pointer_reg] <= 1'b1;
                    buffer[write_pointer_reg] <= data;
                end

                if (state == STREAM) begin
                    if(request_valid && request_ready) begin
                        for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
                            pixel_output[lane] <= decoded_pixel[lane];
                        end

                        response_x <= request_x;
                        response_mask <= request_mask;
                        response_valid <= 1'b1;

                        bit_offset <= advanced_bits % WORD_WIDTH;

                        if(advanced_bits >= WORD_WIDTH) begin
                            word_status[read_pointer] <= 1'b0;
                            read_pointer <= next_read_pointer;

                        end
                    end
                end
            end
            else if (state == DRAIN) begin

            end
        end
    end

    always_comb begin
        read_address = '0;
        read_enable = '0;
        next_state = state;
        prepare_ready = 1'b0;
        request_ready = 1'b0;

        case (state)
            IDLE: begin
                prepare_ready = !reset;

                if (prepare_ready && prepare_valid) begin
                    next_state = LOAD_PALETTE;
                end
            end

            LOAD_PALETTE: begin
                read_enable = !reset && !palette_reads_finished;

                if (read_enable) begin
                    read_address = palette_word_counter;
                end

                if (write_enable && last_palette_response) begin
                    next_state = 
                        (bits_per_color_reg == 0)
                        ? STREAM
                        : BUFFER_FILL;
                end
            end

            BUFFER_FILL: begin
                read_enable = !reset && !word_status[write_pointer] && (pixel_word_counter < words_needed);

                if (read_enable) begin
                    read_address = pixel_word_address;
                end

                if (word_status[0] && ((words_needed == 1) || word_status[1])) begin
                    next_state = STREAM;
                end
            end

            STREAM: begin
                request_ready = !reset && !prepare_valid && buffer_has_data && response_space;

                read_enable = !reset && !prepare_valid && !word_status[write_pointer] && (pixel_word_counter < words_needed);

                if (read_enable) begin
                    read_address = pixel_word_address;
                end

                if (prepare_valid) begin
                    next_state = DRAIN;
                end
            end

            DRAIN: begin
                if (!write_enable && !response_valid) begin
                    next_state = IDLE;
                end
            end

            default: begin
                next_state = IDLE;
            end
        endcase
    end

endmodule