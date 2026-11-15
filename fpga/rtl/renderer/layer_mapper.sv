module layer_mapper
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2
)(
    input logic [X_COORDINATE_WIDTH-1:0] x_counter,
    input layer_descriptor_t current_layer,
    input logic mmu_request_valid,

    output logic [PIXELS_PER_CYCLE-1:0] pixel_group_mask,
    output logic [X_COORDINATE_WIDTH-1:0] destination_x
    );

    assign destination_x = x_counter + (current_layer.dst_x - (current_layer.dst_x % PIXELS_PER_CYCLE));


    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        assign pixel_group_mask[lane] = 
            mmu_request_valid && 
            ((destination_x + lane) >= current_layer.dst_x) &&
            ((destination_x + lane) < current_layer.dst_x + current_layer.width);
    end


endmodule