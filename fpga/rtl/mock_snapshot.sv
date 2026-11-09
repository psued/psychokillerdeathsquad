module mock_snapshot
    import renderer_pkg::*;
(
    output layer_descriptor_t snapshot [0:LAYER_COUNT-1]
);

    always_comb begin
        for (int layer = 0; layer < LAYER_COUNT; layer++) begin
            snapshot[layer] = '0;
        end

        snapshot[0].image_id = 16'd0;
        snapshot[0].width = LINE_PIXELS;
        snapshot[0].height = FRAME_LINES;
        snapshot[0].z = 8'd0;
        snapshot[0].flags = FLAG_ENABLED;

        snapshot[1].image_id = 16'd2;
        snapshot[1].width = 16'd96;
        snapshot[1].height = 16'd64;
        snapshot[1].dst_x = 16'd112;
        snapshot[1].dst_y = 16'd88;
        snapshot[1].z = 8'd1;
        snapshot[1].flags = FLAG_ENABLED;

        snapshot[2].image_id = 16'd1;
        snapshot[2].width = 16'd64;
        snapshot[2].height = 16'd64;
        snapshot[2].dst_x = 16'd80;
        snapshot[2].dst_y = 16'd64;
        snapshot[2].z = 8'd2;
        snapshot[2].flags = FLAG_ENABLED;
    end

endmodule
