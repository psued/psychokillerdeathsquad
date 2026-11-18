module snapshot_cache
    import renderer_pkg::*;
(
    input logic clk,
    input logic reset,
    input logic load_start,
    input logic fetcher_idle,

    input layer_descriptor_t source_layer,
    input logic [15:0] source_asset_width,
    input logic [7:0] source_bits_per_color,

    input logic [LAYER_INDEX_WIDTH-1:0] layer_index,
    input logic [LAYER_INDEX_WIDTH-1:0] fetch_layer_index,
    output logic [LAYER_INDEX_WIDTH-1:0] mmu_layer,

    output layer_descriptor_t current_layer,
    output logic [15:0] asset_width,
    output logic [7:0] bits_per_color,
    output logic ready
);
    layer_descriptor_t layers [0:LAYER_COUNT-1];
    logic [15:0] asset_widths [0:LAYER_COUNT-1];
    logic [7:0] colour_bits [0:LAYER_COUNT-1];
    logic [LAYER_INDEX_WIDTH-1:0] load_index;

    typedef enum logic [1:0] {
        IDLE,
        WAIT_FETCHER,
        LOAD_LAYERS
    } state_t;

    state_t state;

    // Metadata loading owns the MMU selection only after the fetcher is idle.
    assign mmu_layer = (state == LOAD_LAYERS) ? load_index : fetch_layer_index;
    assign current_layer = ready ? layers[layer_index] : '0;
    assign asset_width = ready ? asset_widths[layer_index] : '0;
    assign bits_per_color = ready ? colour_bits[layer_index] : '0;

    always_ff @(posedge clk) begin
        if (reset) begin
            state <= IDLE;
            load_index <= '0;
            ready <= 1'b0;
        end
        else begin
            case (state)
                IDLE: begin
                    if (load_start) begin
                        ready <= 1'b0;
                        load_index <= '0;
                        state <= WAIT_FETCHER;
                    end
                end

                WAIT_FETCHER: begin
                    if (fetcher_idle) begin
                        state <= LOAD_LAYERS;
                    end
                end

                LOAD_LAYERS: begin
                    layers[load_index] <= source_layer;
                    asset_widths[load_index] <= source_asset_width;
                    colour_bits[load_index] <= source_bits_per_color;

                    if (load_index == LAYER_COUNT - 1) begin
                        ready <= 1'b1;
                        state <= IDLE;
                    end
                    else begin
                        load_index <= load_index + 1'b1;
                    end
                end

                default: begin
                    state <= IDLE;
                    ready <= 1'b0;
                end
            endcase
        end
    end
endmodule
