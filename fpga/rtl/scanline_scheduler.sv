module scanline_scheduler
    import renderer_pkg::*;
#(
    parameter PIXELS_PER_CYCLE = 2
)(
    input logic clk,
    input logic reset,

    input logic stall,
    input logic line_done,
    input layer_descriptor_t current_layer,

    output logic [X_COORDINATE_WIDTH-1:0] x_counter,
    output logic [Y_COORDINATE_WIDTH-1:0] y_counter,
    output logic [LAYER_INDEX_WIDTH-1:0] layer_counter,
    output logic mmu_request_valid,
    output logic line_issue_finished,
    output logic frame_done
    );
    //CONTROL LOGIC


    logic [15:0] line_end;
    assign line_end = 
        ((current_layer.width - 1 
        + (current_layer.dst_x % PIXELS_PER_CYCLE))
        / PIXELS_PER_CYCLE) 
        * PIXELS_PER_CYCLE;


    //TODO: Implement layer intersection logic
    logic layer_row_intersect;
    logic layer_disabled;
    logic layer_finished;
    logic layer_skip;
    logic last_layer;

    //STATE MACHINE

    typedef enum logic [2:0] {
        WAIT_BUFFER,
        SELECT_LAYER,
        DRAW,
        DRAIN,
        COMPLETE
    } state_t;

    state_t state;
    state_t next_state;

    always_ff @(posedge clk) begin
        if (reset) begin
            state <= WAIT_BUFFER;
        end
        else begin
            state <= next_state;
        end
    end

    always_comb begin
        next_state = state;

        case (state)
            WAIT_BUFFER: begin
                if (!stall) begin
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
                    next_state = DRAW;
                end
            end

            DRAW: begin 
                if (layer_finished) begin
                    if (last_layer) begin
                        next_state = DRAIN;
                    end
                    else begin
                        next_state = SELECT_LAYER;
                    end
                end
            end

            DRAIN: begin
                if (line_done) begin
                    next_state = COMPLETE;
                end
            end

            COMPLETE: begin
                next_state = WAIT_BUFFER;
            end

            default: begin
                next_state = WAIT_BUFFER;
            end
        endcase
    end

    assign layer_row_intersect = ((y_counter >= current_layer.dst_y) && (y_counter < current_layer.dst_y + current_layer.height));
    assign layer_disabled = ((current_layer.flags & FLAG_ENABLED) == 0);
    assign layer_finished = (x_counter == line_end);
    assign layer_skip = !layer_row_intersect || ((x_counter == 0) && layer_disabled);
    assign last_layer = layer_counter == (LAYER_COUNT-1);
    assign line_issue_finished = !reset && last_layer && (((state == DRAW) && layer_finished) || ((state == SELECT_LAYER) && layer_skip));


    always_ff @(posedge clk) begin
        if (reset) begin
            x_counter <= '0;
            y_counter <= '0;
            layer_counter <= '0;
        end
        else if (state == SELECT_LAYER) begin 
            x_counter <= '0;
            
            if (layer_skip) begin
                if (last_layer) begin
                    layer_counter <= '0;
                end
                else begin
                    layer_counter <= layer_counter + 1'b1;
                end
            end
        end
        else if (state == DRAW) begin
            if (layer_finished) begin
                x_counter <= '0;
                if (!last_layer) begin
                    layer_counter <= layer_counter + 1'b1;
                end
            end
            else begin
                x_counter <= x_counter + PIXELS_PER_CYCLE;
            end
        end
        else if (state == COMPLETE) begin
            x_counter <= '0;
            layer_counter <= '0;
            if (y_counter == FRAME_LINES - 1) begin
                y_counter <= '0;
            end
            else begin
                y_counter <= y_counter + 1'b1;
            end
        end
    end

    assign frame_done = !reset && (state == COMPLETE) && (y_counter == FRAME_LINES - 1);    
    assign mmu_request_valid = !reset && (state == DRAW); 
endmodule