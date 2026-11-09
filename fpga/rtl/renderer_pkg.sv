package renderer_pkg;
    localparam int LINE_PIXELS = 320;
    localparam int FRAME_LINES = 240;
    localparam int X_COORDINATE_WIDTH = $clog2(LINE_PIXELS);
    localparam int Y_COORDINATE_WIDTH = $clog2(FRAME_LINES);

    localparam int CHANNEL_BITS = 4;
    localparam int PIXEL_BITS = 3 * CHANNEL_BITS;
    localparam int PIXEL_SLOT_BITS = 16;
    localparam int IMAGE_ID_WIDTH = 16;

    localparam int LAYER_COUNT = 24;
    localparam int LAYER_INDEX_WIDTH = $clog2(LAYER_COUNT);

    localparam logic [7:0] FLAG_ENABLED = 8'h01;
    localparam logic [7:0] FLAG_FLIP_X = 8'h02;

    typedef logic [PIXEL_BITS-1:0] pixel_t;

    localparam pixel_t TRANSPARENT_COLOR = 12'hF0F;

    typedef struct packed {
        logic [15:0] image_id;
        logic [15:0] src_x;
        logic [15:0] src_y;
        logic [15:0] width;
        logic [15:0] height;
        logic [15:0] dst_x;
        logic [15:0] dst_y;
        logic [7:0]  z;
        logic [7:0]  flags;
    } layer_descriptor_t;

endpackage