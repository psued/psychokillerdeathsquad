module asset_fetcher 
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,

    localparam int WORD_WIDTH = 24,
    localparam int COLOUR_WIDTH = 12,
    localparam int TOTAL_WORDS = 4,
    localparam int MAX_PALETTE_COLOURS = 32,
    localparam int MAX_INDEX_WIDTH = $clog2(MAX_PALETTE_COLOURS),
    localparam int MAX_ASSET_WIDTH = 960,
    localparam int MAX_ASSET_HEIGHT = 320,
    localparam int MAX_PIXELS = MAX_ASSET_WIDTH * MAX_ASSET_HEIGHT,
    localparam int MAX_PIXEL_WORDS = (MAX_PIXELS * MAX_INDEX_WIDTH + WORD_WIDTH - 1) / WORD_WIDTH,
    localparam int PIXEL_BIT_WIDTH = $clog2(MAX_PIXELS * MAX_INDEX_WIDTH),
    localparam int SETUP_LAST_CYCLE = PIXEL_BIT_WIDTH + 4
)(
    input logic clk,
    input logic reset,
    input logic pause,
    output logic idle,

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
    (* MARK_DEBUG = "TRUE" *) logic [15:0] width_reg;
    logic [15:0] asset_width_reg;
    logic [7:0] bits_per_color_reg;
    logic flipped_reg;

    logic [$clog2(MAX_PALETTE_COLOURS*COLOUR_WIDTH/WORD_WIDTH):0] palette_words;


    typedef enum logic [2:0] {
        IDLE,
        SETUP,
        LOAD_PALETTE,
        BUFFER_FILL,
        STREAM,
        DRAIN
    } state_t;

    (* MARK_DEBUG = "TRUE" *) state_t state;
    state_t next_state;
    logic [$clog2(SETUP_LAST_CYCLE+1)-1:0] setup_counter;

    always_ff @(posedge clk) begin
        if (reset) begin
            state <= IDLE;
        end
        else begin
            state <= next_state;
        end
    end

    logic [15:0] read_address_reg;
    logic [WORD_WIDTH-1:0] read_data_reg;
    logic [4:0] active_layer;

    assign mmu_layer = active_layer;
    assign idle = !reset && (state == IDLE);

    logic [11:0] palette [0:31];
    logic [$clog2(MAX_PALETTE_COLOURS)-2:0] palette_word_counter;    

    logic read_enable;
    logic read_pending;
    logic write_enable;
    logic [15:0] write_address;
    logic palette_reads_finished;
    logic last_palette_request;
    logic last_palette_response;

    assign last_palette_request = palette_word_counter == (palette_words - 1);
    assign last_palette_response = (write_address == (palette_words - 1));

    
    //BUFFER FILL LOGIC
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] pixel_word_counter;

    logic [15:0] asset_y;
    logic [$clog2(MAX_PIXELS)-1:0] first_pixel_index;
    logic [PIXEL_BIT_WIDTH-1:0] first_pixel_bit;
    logic [PIXEL_BIT_WIDTH-1:0] pixel_span_bits;
    logic [PIXEL_BIT_WIDTH-1:0] first_word_quotient;
    logic [PIXEL_BIT_WIDTH-1:0] last_word_quotient;
    logic [$clog2(WORD_WIDTH)-1:0] first_remainder;
    logic [$clog2(WORD_WIDTH)-1:0] last_remainder;
    logic [$clog2(WORD_WIDTH):0] first_divide_bits;
    logic [$clog2(WORD_WIDTH):0] last_divide_bits;
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] first_word_address;
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] last_word_address;
    logic [15:0] pixel_word_address;
    logic [$clog2(WORD_WIDTH)-1:0] initial_bit_offset;
    logic [$clog2(WORD_WIDTH)-1:0] bit_offset;
    logic [$clog2(MAX_PIXEL_WORDS)-1:0] words_needed;

    assign first_divide_bits = {first_remainder, first_word_quotient[PIXEL_BIT_WIDTH-1]};
    assign last_divide_bits = {last_remainder, last_word_quotient[PIXEL_BIT_WIDTH-1]};

    logic [WORD_WIDTH-1:0] buffer [0:TOTAL_WORDS-1];
    logic [TOTAL_WORDS-1:0] word_status;
    logic [$clog2(TOTAL_WORDS)-1:0] read_pointer;
    logic [$clog2(TOTAL_WORDS)-1:0] write_pointer;
    logic [$clog2(TOTAL_WORDS)-1:0] write_pointer_reg;
    logic [$clog2(TOTAL_WORDS)-1:0] buffer_write_slot;

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
    logic request_stage_valid;
    logic [X_COORDINATE_WIDTH-1:0] request_stage_x;
    logic [PIXELS_PER_CYCLE-1:0] request_stage_mask;

    logic index_stage_valid;
    logic [MAX_INDEX_WIDTH-1:0] index_stage [0:PIXELS_PER_CYCLE-1];
    logic [X_COORDINATE_WIDTH-1:0] index_stage_x;
    logic [PIXELS_PER_CYCLE-1:0] index_stage_mask;

    int unsigned bits_consumed;
    int unsigned shift_amount;
    int unsigned advanced_bits;

    always_comb begin
        bits_consumed = 0;
        shift_amount = 0;

        for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
            pixel_index[lane] = '0;

            if (request_stage_mask[lane] && (bits_per_color_reg != 0)) begin
                shift_amount = flipped_reg
                             ? bit_offset + bits_consumed
                             : 2*WORD_WIDTH - bit_offset - bits_consumed - bits_per_color_reg;
                pixel_index[lane] = (pixel_window >> shift_amount) & ((1 << bits_per_color_reg) - 1);
                bits_consumed = bits_consumed + bits_per_color_reg;
            end
        end
    end

    assign advanced_bits = bit_offset + bits_consumed;

    logic buffer_has_data;
    logic response_space;
    logic index_space;
    logic request_space;
    logic pipeline_active;
    logic request_fire;
    logic decode_fire;

    assign buffer_has_data = 
        (bits_consumed == 0) || 
        (word_status[read_pointer] &&
        ((bit_offset + bits_consumed <= WORD_WIDTH) ||
        word_status[next_read_pointer]));

    assign response_space = !response_valid || response_ready;
    assign index_space = !index_stage_valid || response_space;
    assign pipeline_active = (state == STREAM) || (state == DRAIN);
    assign decode_fire = !reset && pipeline_active && request_stage_valid
                      && buffer_has_data && index_space;
    assign request_space = !request_stage_valid || decode_fire;
    assign request_fire = request_valid && request_ready;

    // Keep each returned word aligned with its address and reserved buffer slot.
    always_ff @(posedge clk) begin
        if (reset) begin
            read_address_reg <= '0;
            write_address <= '0;
            read_data_reg <= '0;
            buffer_write_slot <= '0;
        end
        else begin
            read_address_reg <= read_address;
            write_address <= read_address_reg;
            read_data_reg <= data;
            buffer_write_slot <= write_pointer_reg;
        end
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            active_layer <= '0;
            setup_counter <= '0;
            flipped_reg <= 1'b0;
            palette_word_counter <= '0;
            palette_words <= '0;
            read_pending <= 1'b0;
            write_enable <= '0;
            palette_reads_finished <= '0;

            word_status <= '0;
            read_pointer <= '0;
            write_pointer <= '0;
            write_pointer_reg <= '0;
            response_valid <= '0;
            request_stage_valid <= 1'b0;
            index_stage_valid <= 1'b0;

        end
        else begin
            read_pending <= read_enable;
            write_enable <= read_pending;

            // Transfer each stage only when its destination has space.
            if (response_space) begin
                response_valid <= index_stage_valid;
                if (index_stage_valid) begin
                    for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
                        pixel_output[lane] <= index_stage_mask[lane]
                                           ? palette[index_stage[lane]] : '0;
                    end
                    response_x <= index_stage_x;
                    response_mask <= index_stage_mask;
                end
            end

            if (index_space) begin
                index_stage_valid <= decode_fire;
                if (decode_fire) begin
                    for (int lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
                        index_stage[lane] <= pixel_index[lane];
                    end
                    index_stage_x <= request_stage_x;
                    index_stage_mask <= request_stage_mask;
                end
            end

            if (request_fire) begin
                request_stage_valid <= 1'b1;
                request_stage_x <= request_x;
                request_stage_mask <= request_mask;
            end
            else if (decode_fire) begin
                request_stage_valid <= 1'b0;
            end

            if(state == IDLE) begin
                if(prepare_valid && prepare_ready) begin
                    active_layer <= layer;
                    setup_counter <= '0;
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
            else if (state == SETUP) begin
                if (setup_counter < SETUP_LAST_CYCLE) begin
                    setup_counter <= setup_counter + 1'b1;
                end

                case (setup_counter)
                    0: begin
                        asset_y <= src_y_reg + y_select_reg - dst_y_reg;
                        case (bits_per_color_reg)
                            8'd0, 8'd1: palette_words <= 5'd1;
                            8'd2: palette_words <= 5'd2;
                            8'd3: palette_words <= 5'd4;
                            8'd4: palette_words <= 5'd8;
                            8'd5: palette_words <= 5'd16;
                            default: palette_words <= 5'd1;
                        endcase
                    end
                    1: begin
                        first_pixel_index <= asset_y * asset_width_reg + src_x_reg;
                    end
                    2: begin
                        first_pixel_bit <= first_pixel_index * bits_per_color_reg;
                        pixel_span_bits <= width_reg * bits_per_color_reg;
                    end
                    3: begin
                        first_word_quotient <= first_pixel_bit;
                        last_word_quotient <= (bits_per_color_reg == 0)
                                            ? '0 : first_pixel_bit + pixel_span_bits - 1'b1;
                        first_remainder <= '0;
                        last_remainder <= '0;
                    end
                    SETUP_LAST_CYCLE: begin
                        first_word_address <= palette_words + first_word_quotient;
                        last_word_address <= palette_words + last_word_quotient;
                        initial_bit_offset <= (bits_per_color_reg == 0) ? '0
                                            : (flipped_reg
                                               ? WORD_WIDTH - 1 - last_remainder
                                               : first_remainder);
                        words_needed <= (bits_per_color_reg == 0) ? '0
                                      : last_word_quotient - first_word_quotient + 1'b1;
                    end
                    default: begin
                        // Divide both bit addresses by WORD_WIDTH, one bit per clock.
                        first_word_quotient <= {first_word_quotient[PIXEL_BIT_WIDTH-2:0],
                                               (first_divide_bits >= WORD_WIDTH)};
                        last_word_quotient <= {last_word_quotient[PIXEL_BIT_WIDTH-2:0],
                                              (last_divide_bits >= WORD_WIDTH)};
                        first_remainder <= (first_divide_bits >= WORD_WIDTH)
                                         ? first_divide_bits - WORD_WIDTH : first_divide_bits;
                        last_remainder <= (last_divide_bits >= WORD_WIDTH)
                                        ? last_divide_bits - WORD_WIDTH : last_divide_bits;
                    end
                endcase
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
                    palette[write_address*2] <= read_data_reg[23:12];
                    palette[write_address*2 + 1] <= read_data_reg[11:0];
                end

                if(write_enable && last_palette_response) begin
                    write_pointer <= '0;
                    write_pointer_reg <= '0;
                    read_pointer <= '0;
                    bit_offset <= initial_bit_offset;
                    word_status <= '0;
                    pixel_word_counter <= '0;
                    pixel_word_address <= flipped_reg
                                        ? last_word_address : first_word_address;
                end
            end
            else if (state == BUFFER_FILL || state == STREAM || state == DRAIN) begin
                write_pointer_reg <= write_pointer;
                if (read_enable) begin
                    write_pointer <= write_pointer + 1'b1;
                    pixel_word_counter <= pixel_word_counter + 1'b1;
                    pixel_word_address <= flipped_reg
                                        ? pixel_word_address - 1'b1
                                        : pixel_word_address + 1'b1;
                end

                if(write_enable) begin
                    word_status[buffer_write_slot] <= 1'b1;
                    buffer[buffer_write_slot] <= read_data_reg;
                end

                if (decode_fire) begin
                    bit_offset <= advanced_bits % WORD_WIDTH;

                    if (advanced_bits >= WORD_WIDTH) begin
                        word_status[read_pointer] <= 1'b0;
                        read_pointer <= next_read_pointer;
                    end
                end
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
                prepare_ready = !reset && !pause;

                if (prepare_ready && prepare_valid) begin
                    next_state = SETUP;
                end
            end

            SETUP: begin
                if (setup_counter == SETUP_LAST_CYCLE) begin
                    next_state = LOAD_PALETTE;
                end
            end

            LOAD_PALETTE: begin
                read_enable = !reset && !palette_reads_finished;

                read_address = palette_word_counter;

                if (write_enable && last_palette_response) begin
                    next_state = 
                        (bits_per_color_reg == 0)
                        ? STREAM
                        : BUFFER_FILL;
                end
            end

            BUFFER_FILL: begin
                read_enable = !reset && !word_status[write_pointer] && (pixel_word_counter < words_needed);

                read_address = pixel_word_address;

                if (word_status[0] && ((words_needed == 1) || word_status[1])) begin
                    next_state = STREAM;
                end
            end

            STREAM: begin
                request_ready = !reset && !pause && !prepare_valid && request_space;

                read_enable = !reset && !pause && !word_status[write_pointer]
                           && (pixel_word_counter < words_needed);
                read_address = pixel_word_address;

                if (pause || prepare_valid) begin
                    next_state = DRAIN;
                end
            end

            DRAIN: begin
                read_enable = !reset && request_stage_valid
                           && !word_status[write_pointer] && (pixel_word_counter < words_needed);
                read_address = pixel_word_address;

                if (!read_pending && !write_enable && !request_stage_valid
                    && !index_stage_valid && !response_valid) begin
                    next_state = IDLE;
                end
            end

            default: begin
                next_state = IDLE;
            end
        endcase
    end

endmodule
