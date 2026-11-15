module scanline_scheduler
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2,
    localparam int LAST_CLEAR_X = ((LINE_PIXELS - 1) / PIXELS_PER_CYCLE) * PIXELS_PER_CYCLE
)(
    input logic clk,
    input logic reset,

    input logic stall,
    input logic writeback_empty,
    input logic clear_ready,
    input logic sort_done,
    input logic mmu_prepare_ready,
    input logic mmu_request_ready,
    input layer_descriptor_t current_layer,

    output logic [X_COORDINATE_WIDTH-1:0] x_counter,
    output logic [Y_COORDINATE_WIDTH-1:0] y_counter,
    output logic [LAYER_INDEX_WIDTH-1:0] layer_order_counter,
    output logic sort_start,
    output logic frame_setup,
    output logic clear_valid,
    output logic mmu_prepare_valid,
    output logic mmu_request_valid,
    output logic line_done,
    output logic frame_done
    );
    //CONTROL LOGIC


    logic [15:0] line_end;
    assign line_end =
        (current_layer.width == 0) ? '0 :
        ((current_layer.width - 1 
        + (current_layer.dst_x % PIXELS_PER_CYCLE))
        / PIXELS_PER_CYCLE) 
        * PIXELS_PER_CYCLE;


    logic layer_row_intersect;
    logic layer_disabled;
    logic layer_finished;
    logic layer_skip;
    logic last_layer;
    logic request_fire;
    logic clear_fire;

    //STATE MACHINE

    typedef enum logic [3:0] {
        START_FRAME,
        WAIT_SORT,
        WAIT_BUFFER,
        CLEAR_LINE,
        SELECT_LAYER,
        PREPARE_LAYER,
        DRAW,
        DRAIN,
        COMPLETE
    } state_t;

    state_t state;
    state_t next_state;

    always_ff @(posedge clk) begin
        if (reset) begin
            state <= START_FRAME;
        end
        else begin
            state <= next_state;
        end
    end

    always_comb begin
        next_state = state;

        case (state)
            START_FRAME: begin
                next_state = WAIT_SORT;
            end

            WAIT_SORT: begin
                if (sort_done) begin
                    next_state = WAIT_BUFFER;
                end
            end

            WAIT_BUFFER: begin
                if (!stall) begin
                    next_state = CLEAR_LINE;
                end
            end

            CLEAR_LINE: begin
                if (clear_fire && (x_counter == LAST_CLEAR_X)) begin
                    next_state = SELECT_LAYER;
                end
            end

            SELECT_LAYER: begin
                if (layer_skip) begin
                    if (last_layer) begin
                        next_state = DRAIN;
                    end
                end
                else begin
                    next_state = PREPARE_LAYER;
                end
            end

            PREPARE_LAYER: begin
                if (mmu_prepare_valid && mmu_prepare_ready) begin
                    next_state = DRAW;
                end
            end

            DRAW: begin
                if (request_fire && layer_finished) begin
                    if (last_layer) begin
                        next_state = DRAIN;
                    end
                    else begin
                        next_state = SELECT_LAYER;
                    end
                end
            end

            DRAIN: begin
                if (writeback_empty) begin
                    next_state = COMPLETE;
                end
            end

            COMPLETE: begin
                next_state = (y_counter == FRAME_LINES - 1)
                           ? START_FRAME
                           : WAIT_BUFFER;
            end

            default: begin
                next_state = START_FRAME;
            end
        endcase
    end

    assign layer_row_intersect = ((y_counter >= current_layer.dst_y) && (y_counter < current_layer.dst_y + current_layer.height));
    assign layer_disabled = ((current_layer.flags & FLAG_ENABLED) == 0);
    assign layer_finished = (x_counter == line_end);
    assign layer_skip = layer_disabled || !layer_row_intersect || (current_layer.width == 0);
    assign last_layer = layer_order_counter == (LAYER_COUNT-1);
    assign request_fire = mmu_request_valid && mmu_request_ready;
    assign clear_fire = clear_valid && clear_ready;


    always_ff @(posedge clk) begin
        if (reset) begin
            x_counter <= '0;
            y_counter <= '0;
            layer_order_counter <= '0;
        end
        else if ((state == CLEAR_LINE) && clear_fire) begin
            if (x_counter == LAST_CLEAR_X) begin
                x_counter <= '0;
            end
            else begin
                x_counter <= x_counter + PIXELS_PER_CYCLE;
            end
        end
        else if (state == SELECT_LAYER) begin 
            x_counter <= '0;
            
            if (layer_skip) begin
                if (last_layer) begin
                    layer_order_counter <= '0;
                end
                else begin
                    layer_order_counter <= layer_order_counter + 1'b1;
                end
            end
        end
        else if ((state == DRAW) && request_fire) begin
            if (layer_finished) begin
                x_counter <= '0;
                if (!last_layer) begin
                    layer_order_counter <= layer_order_counter + 1'b1;
                end
            end
            else begin
                x_counter <= x_counter + PIXELS_PER_CYCLE;
            end
        end
        else if (state == COMPLETE) begin
            x_counter <= '0;
            layer_order_counter <= '0;
            if (y_counter == FRAME_LINES - 1) begin
                y_counter <= '0;
            end
            else begin
                y_counter <= y_counter + 1'b1;
            end
        end
    end

    assign line_done = !reset && (state == COMPLETE);
    assign frame_done = line_done && (y_counter == FRAME_LINES - 1);
    assign sort_start = !reset && (state == START_FRAME);
    assign frame_setup = (state == START_FRAME) || (state == WAIT_SORT);
    assign clear_valid = !reset && (state == CLEAR_LINE);
    assign mmu_prepare_valid = !reset && (state == PREPARE_LAYER);
    assign mmu_request_valid = !reset && (state == DRAW);
endmodule