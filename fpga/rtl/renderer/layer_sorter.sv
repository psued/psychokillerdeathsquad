module layer_sorter
    import renderer_pkg::*;
(
    input logic clk,
    input logic reset,
    input logic start,
    input logic [7:0] layer_z,
    input logic [LAYER_INDEX_WIDTH-1:0] order_index,

    output logic [LAYER_INDEX_WIDTH-1:0] read_layer_index,
    output logic [LAYER_INDEX_WIDTH-1:0] sorted_layer_index,
    output logic done
);
    typedef struct packed {
        logic [7:0] z;
        logic [LAYER_INDEX_WIDTH-1:0] slot_index;
    } sort_entry_t;

    sort_entry_t order_table [0:LAYER_COUNT-1];
    sort_entry_t key;

    logic [LAYER_INDEX_WIDTH-1:0] load_counter;
    logic [LAYER_INDEX_WIDTH-1:0] sort_counter;
    logic [LAYER_INDEX_WIDTH-1:0] insert_index;
    logic shift_required;

    typedef enum logic [2:0] {
        SORT_IDLE,
        LOAD_Z,
        LOAD_KEY,
        SHIFT,
        INSERT,
        SORT_DONE
    } state_t;

    state_t state;

    assign read_layer_index = load_counter;
    assign done = (state == SORT_DONE);
    assign sorted_layer_index = done
                              ? order_table[order_index].slot_index
                              : '0;

    always_comb begin
        shift_required = 1'b0;
        if (insert_index != 0) begin
            shift_required = order_table[insert_index - 1'b1].z > key.z;
        end
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            state <= SORT_IDLE;
            load_counter <= '0;
            sort_counter <= '0;
            insert_index <= '0;
            key <= '0;
        end
        else begin
            case (state)
                SORT_IDLE, SORT_DONE: begin
                    if (start) begin
                        load_counter <= '0;
                        state <= LOAD_Z;
                    end
                end

                LOAD_Z: begin
                    order_table[load_counter].z <= layer_z;
                    order_table[load_counter].slot_index <= load_counter;

                    if (load_counter == LAYER_COUNT - 1) begin
                        sort_counter <= 1'b1;
                        state <= LOAD_KEY;
                    end
                    else begin
                        load_counter <= load_counter + 1'b1;
                    end
                end

                LOAD_KEY: begin
                    key <= order_table[sort_counter];
                    insert_index <= sort_counter;
                    state <= SHIFT;
                end

                SHIFT: begin
                    if (shift_required) begin
                        order_table[insert_index] <= order_table[insert_index - 1'b1];
                        insert_index <= insert_index - 1'b1;
                    end
                    else begin
                        state <= INSERT;
                    end
                end

                INSERT: begin
                    order_table[insert_index] <= key;
                    if (sort_counter == LAYER_COUNT - 1) begin
                        state <= SORT_DONE;
                    end
                    else begin
                        sort_counter <= sort_counter + 1'b1;
                        state <= LOAD_KEY;
                    end
                end

                default: begin
                    state <= SORT_IDLE;
                end
            endcase
        end
    end
endmodule
