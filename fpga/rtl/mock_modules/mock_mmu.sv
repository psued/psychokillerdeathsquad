module mock_mmu
    import renderer_pkg::*;
(
    input logic clk,
    input logic reset,
    input logic [LAYER_INDEX_WIDTH-1:0] layer,
    input logic [15:0] read_address,

    output layer_descriptor_t current_layer,
    output logic [15:0] asset_width,
    output logic [7:0] bits_per_color,
    output logic [23:0] data
);
    localparam int WORD_BITS = 24;
    localparam int CHECKER_SIZE = 64;
    localparam int CHECKER_WORDS =
        1 + (CHECKER_SIZE * CHECKER_SIZE + WORD_BITS - 1) / WORD_BITS;
    localparam int GRADIENT_WIDTH = 96;
    localparam int GRADIENT_HEIGHT = 64;
    localparam int GRADIENT_INDEX_BITS = 5;
    localparam int GRADIENT_PALETTE_WORDS = 16;
    localparam int GRADIENT_WORDS = GRADIENT_PALETTE_WORDS +
        (GRADIENT_WIDTH * GRADIENT_HEIGHT * GRADIENT_INDEX_BITS
         + WORD_BITS - 1) / WORD_BITS;

    logic [WORD_BITS-1:0] checker_rom [0:CHECKER_WORDS-1];
    logic [WORD_BITS-1:0] gradient_rom [0:GRADIENT_WORDS-1];

    mock_snapshot snapshot_source (
        .layer_counter(layer),
        .current_layer(current_layer)
    );

    always_comb begin
        asset_width = '0;
        bits_per_color = '0;

        case (current_layer.image_id)
            16'd0: asset_width = LINE_PIXELS;
            16'd1: begin
                asset_width = CHECKER_SIZE;
                bits_per_color = 8'd1;
            end
            16'd2: begin
                asset_width = GRADIENT_WIDTH;
                bits_per_color = GRADIENT_INDEX_BITS;
            end
            default: begin end
        endcase
    end

    // Eight red levels and four green levels fit a 32-colour palette.
    function automatic pixel_t gradient_colour(input int unsigned index_value);
        gradient_colour = {
            4'(((index_value / 4) * 15) / 7),
            4'((index_value % 4) * 5),
            4'h4
        };
    endfunction

    function automatic logic [WORD_BITS-1:0] make_checker_word(
        input int unsigned address
    );
        int unsigned pixel_number;
        int unsigned source_x;
        int unsigned source_y;

        make_checker_word = '0;
        if (address == 0) begin
            make_checker_word = {TRANSPARENT_COLOR, 12'hFFF};
        end
        else begin
            for (int bit_number = 0; bit_number < WORD_BITS; bit_number++) begin
                pixel_number = (address - 1) * WORD_BITS + bit_number;
                source_x = pixel_number % CHECKER_SIZE;
                source_y = pixel_number / CHECKER_SIZE;
                if (pixel_number < CHECKER_SIZE * CHECKER_SIZE)
                    make_checker_word[WORD_BITS-1-bit_number] =
                        ((source_x / 8) ^ (source_y / 8)) & 1;
            end
        end
    endfunction

    function automatic logic [WORD_BITS-1:0] make_gradient_word(
        input int unsigned address
    );
        int unsigned stream_bit;
        int unsigned pixel_number;
        int unsigned source_x;
        int unsigned source_y;
        logic [GRADIENT_INDEX_BITS-1:0] index_value;

        make_gradient_word = '0;
        if (address < GRADIENT_PALETTE_WORDS) begin
            make_gradient_word = {
                gradient_colour(address * 2),
                gradient_colour(address * 2 + 1)
            };
        end
        else begin
            for (int bit_number = 0; bit_number < WORD_BITS; bit_number++) begin
                stream_bit = (address - GRADIENT_PALETTE_WORDS)
                           * WORD_BITS + bit_number;
                pixel_number = stream_bit / GRADIENT_INDEX_BITS;
                source_x = pixel_number % GRADIENT_WIDTH;
                source_y = pixel_number / GRADIENT_WIDTH;
                index_value = GRADIENT_INDEX_BITS'(
                    ((source_x * 8) / GRADIENT_WIDTH) * 4
                    + (source_y * 4) / GRADIENT_HEIGHT);
                if (pixel_number < GRADIENT_WIDTH * GRADIENT_HEIGHT)
                    make_gradient_word[WORD_BITS-1-bit_number] =
                        index_value[GRADIENT_INDEX_BITS-1
                                    - (stream_bit % GRADIENT_INDEX_BITS)];
            end
        end
    endfunction

    // Constant contents are generated once and synthesize as ROM.
    initial begin
        for (int address = 0; address < CHECKER_WORDS; address++)
            checker_rom[address] = make_checker_word(address);
        for (int address = 0; address < GRADIENT_WORDS; address++)
            gradient_rom[address] = make_gradient_word(address);
    end

    // Match the MMU's one-clock, 24-bit word-read interface.
    always_ff @(posedge clk) begin
        if (reset) begin
            data <= '0;
        end
        else begin
            data <= '0;
            case (current_layer.image_id)
                16'd0: begin
                    if (read_address == 0)
                        data <= {12'h124, 12'h000};
                end
                16'd1: begin
                    if (read_address < CHECKER_WORDS)
                        data <= checker_rom[read_address];
                end
                16'd2: begin
                    if (read_address < GRADIENT_WORDS)
                        data <= gradient_rom[read_address];
                end
                default: begin
                    if (read_address == 0)
                        data <= {TRANSPARENT_COLOR, 12'h000};
                end
            endcase
        end
    end
endmodule