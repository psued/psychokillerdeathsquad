module layer_mapper
    import renderer_pkg::*;
#(
    parameter int PIXELS_PER_CYCLE = 2
)(
    input logic [X_COORDINATE_WIDTH-1:0] x_counter,
    input logic [Y_COORDINATE_WIDTH-1:0] y_counter,
    input layer_descriptor_t current_layer,
    input logic mmu_request_valid,

    output logic [IMAGE_ID_WIDTH-1:0] mmu_request_image_id,
    output logic [PIXEL_SLOT_BITS-1:0] mmu_request_x [0:PIXELS_PER_CYCLE-1],
    output logic [PIXEL_SLOT_BITS-1:0] mmu_request_y [0:PIXELS_PER_CYCLE-1],
    output logic [PIXELS_PER_CYCLE-1:0] pixel_group_mask,
    output logic [X_COORDINATE_WIDTH-1:0] destination_x
    );

    assign destination_x = x_counter + (current_layer.dst_x - (current_layer.dst_x % PIXELS_PER_CYCLE));
    assign mmu_request_image_id = current_layer.image_id;


    for (genvar lane = 0; lane < PIXELS_PER_CYCLE; lane++) begin
        logic [15:0] pixel_offset;
        assign pixel_offset = destination_x + lane - current_layer.dst_x;

        assign mmu_request_x[lane] = 
            ((current_layer.flags & FLAG_FLIP_X) != 0) 
            ? current_layer.src_x + current_layer.width - 1 - pixel_offset
            : current_layer.src_x + pixel_offset;

        assign mmu_request_y[lane] = y_counter - current_layer.dst_y + current_layer.src_y;
        
        assign pixel_group_mask[lane] = 
            mmu_request_valid && 
            ((destination_x + lane) >= current_layer.dst_x) &&
            ((destination_x + lane) < current_layer.dst_x + current_layer.width);
    end


endmodule