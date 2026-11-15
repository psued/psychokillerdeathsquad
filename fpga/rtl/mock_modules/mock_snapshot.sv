module mock_snapshot
    import renderer_pkg::*;
(
    input logic [LAYER_INDEX_WIDTH-1:0] layer_counter,
    output layer_descriptor_t current_layer
);

    always_comb begin
        current_layer = '0;

        case (layer_counter)
            5'd0: begin
                current_layer.image_id = 16'd0;
                current_layer.width = LINE_PIXELS;
                current_layer.height = FRAME_LINES;
                current_layer.z = 8'd0;
                current_layer.flags = '0;
            end

            5'd1: begin
                current_layer.image_id = 16'd2;
                current_layer.width = 16'd96;
                current_layer.height = 16'd64;
                current_layer.dst_x = 16'd112;
                current_layer.dst_y = 16'd88;
                current_layer.z = 8'd1;
                current_layer.flags = FLAG_ENABLED;
            end

            5'd2: begin
                current_layer.image_id = 16'd1;
                current_layer.width = 16'd64;
                current_layer.height = 16'd64;
                current_layer.dst_x = 16'd80;
                current_layer.dst_y = 16'd64;
                current_layer.z = 8'd2;
                current_layer.flags = FLAG_ENABLED;
            end

            default: current_layer = '0;
        endcase
    end

endmodule
