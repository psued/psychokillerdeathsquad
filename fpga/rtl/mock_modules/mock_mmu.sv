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
    localparam int BACKGROUND_WORDS = 24008;
    localparam int CHARACTER_WORDS = 14950;
    localparam int TEXT_WORDS = 3628;

    (* rom_style = "block" *) logic [23:0] background_rom [0:BACKGROUND_WORDS-1];
    (* rom_style = "block" *) logic [23:0] character_rom [0:CHARACTER_WORDS-1];
    (* rom_style = "block" *) logic [23:0] text_rom [0:TEXT_WORDS-1];

    logic [23:0] background_data;
    logic [23:0] character_data;
    logic [23:0] text_data;
    logic [15:0] read_image_id;
    logic background_address_valid;
    logic character_address_valid;
    logic text_address_valid;
    logic address_zero;
    logic read_reset;

    mock_snapshot snapshot_source (
        .layer_counter(layer),
        .current_layer(current_layer)
    );

    always_comb begin
        asset_width = '0;
        bits_per_color = '0;
        case (current_layer.image_id)
            16'd0: begin
                asset_width = 16'd600;
                bits_per_color = 8'd4;
            end
            16'd1: begin
                asset_width = 16'd560;
                bits_per_color = 8'd5;
            end
            16'd2: begin
                asset_width = 16'd272;
                bits_per_color = 8'd1;
            end
            default: begin end
        endcase
    end

    initial begin
        $readmemh("chinabg.mem", background_rom, 0, BACKGROUND_WORDS-1);
        $readmemh("fredrikidle_comp.mem", character_rom, 0, CHARACTER_WORDS-1);
        $readmemh("roundovertext.mem", text_rom, 0, TEXT_WORDS-1);
    end

    // Register each ROM read before selecting the returned asset.
    always_ff @(posedge clk) begin
        background_data <= background_rom[read_address];
        character_data <= character_rom[read_address];
        text_data <= text_rom[read_address];
        read_image_id <= current_layer.image_id;
        background_address_valid <= read_address < BACKGROUND_WORDS;
        character_address_valid <= read_address < CHARACTER_WORDS;
        text_address_valid <= read_address < TEXT_WORDS;
        address_zero <= read_address == 0;
        read_reset <= reset;
    end

    always_comb begin
        data = '0;
        if (!read_reset) begin
            case (read_image_id)
                16'd0: if (background_address_valid) data = background_data;
                16'd1: if (character_address_valid) data = character_data;
                16'd2: if (text_address_valid) data = text_data;
                default: if (address_zero) data = {TRANSPARENT_COLOR, 12'h000};
            endcase
        end
    end
endmodule