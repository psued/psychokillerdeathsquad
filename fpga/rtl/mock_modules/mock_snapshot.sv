module mock_snapshot
    import renderer_pkg::*;
(
    input logic [LAYER_INDEX_WIDTH-1:0] layer_counter,
    output layer_descriptor_t current_layer
);

    always_comb begin
        current_layer = '0;

        // Slots are deliberately out of z order.
        case (layer_counter)
            5'd0: begin
                current_layer.image_id = 16'd2;
                current_layer.src_y = 16'd128;
                current_layer.width = 16'd272;
                current_layer.height = 16'd64;
                current_layer.dst_x = 16'd24;
                current_layer.dst_y = 16'd8;
                current_layer.z = 8'd3;
                current_layer.flags = FLAG_ENABLED;
            end

            5'd1: begin
                current_layer.image_id = 16'd1;
                current_layer.width = 16'd80;
                current_layer.height = 16'd128;
                current_layer.dst_x = 16'd40;
                current_layer.dst_y = 16'd104;
                current_layer.z = 8'd1;
                current_layer.flags = FLAG_ENABLED;
            end

            5'd2: begin
                current_layer.image_id = 16'd0;
                current_layer.src_x = 16'd140;
                current_layer.width = 16'd320;
                current_layer.height = 16'd240;
                current_layer.z = 8'd0;
                current_layer.flags = FLAG_ENABLED;
            end

            5'd3: begin
                current_layer.image_id = 16'd1;
                current_layer.width = 16'd80;
                current_layer.height = 16'd128;
                current_layer.dst_x = 16'd201;
                current_layer.dst_y = 16'd104;
                current_layer.z = 8'd2;
                current_layer.flags = FLAG_ENABLED | FLAG_FLIP_X;
            end

            default: current_layer = '0;
        endcase
    end

endmodule
